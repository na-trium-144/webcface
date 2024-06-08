#include "data_store2.h"
#include "webcface/field.h"
#include <type_traits>

WEBCFACE_NS_BEGIN
namespace Internal {
/*!
 * \brief setSend時にこれを実際に送信すべきかどうか
 *
 */
template <typename T>
static bool shouldSend(const T &prev, const T &current) {
    if constexpr (std::is_same_v<T, ValueData> || std::is_same_v<T, TextData>) {
        return *prev != *current;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return prev != current;
    } else if constexpr (std::is_same_v<T, FuncData>) {
        // Funcは内容が変更されても2回目以降送信しない
        return false;
    } else {
        return true;
    }
}

template <typename T, typename ReqT>
SyncDataStore2<T, ReqT>::SyncDataStore2(const std::u8string &name)
    : self_member_name(name) {}

template <typename T, typename ReqT>
bool SyncDataStore2<T, ReqT>::isSelf(std::u8string_view member) const {
    return member == self_member_name;
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setSend(const FieldBase &base, const T &data) {
    setSend(base.field_, data);
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setSend(const std::u8string &name,
                                      const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    // auto &recv_self = data_recv[self_member_name];
    // recv_self[name] = data; // 送信後に自分の値を参照する用
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setRecv(const FieldBase &base, const T &data) {
    setRecv(base.member_, base.field_, data);
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setRecv(const std::u8string &from,
                                      const std::u8string &name,
                                      const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T, typename ReqT>
std::unordered_set<std::u8string>
SyncDataStore2<T, ReqT>::getEntry(const FieldBase &base) {
    return getEntry(base.member_);
}
template <typename T, typename ReqT>
std::unordered_set<std::u8string>
SyncDataStore2<T, ReqT>::getEntry(const std::u8string &name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return std::unordered_set<std::u8string>{};
    }
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::clearEntry(const std::u8string &from) {
    std::lock_guard lock(mtx);
    entry[from].clear();
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setEntry(const std::u8string &from,
                                       const std::u8string &e) {
    std::lock_guard lock(mtx);
    entry[from].emplace(e);
}

template <typename T, typename ReqT>
unsigned int SyncDataStore2<T, ReqT>::addReq(const std::u8string &member,
                                             const std::u8string &field) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && req[member][field] == 0) {
        unsigned int max_req = 0;
        for (const auto &r : req) {
            for (const auto &r2 : r.second) {
                if (r2.second > max_req) {
                    max_req = r2.second;
                }
            }
        }
        req[member][field] = max_req + 1;
        return max_req + 1;
    }
    return 0;
}
template <typename T, typename ReqT>
unsigned int SyncDataStore2<T, ReqT>::addReq(const std::u8string &member,
                                             const std::u8string &field,
                                             const ReqT &req_info) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && (req[member][field] == 0 ||
                            this->req_info[member][field] != req_info)) {
        unsigned int max_req = 0;
        for (const auto &r : req) {
            for (const auto &r2 : r.second) {
                if (r2.second > max_req) {
                    max_req = r2.second;
                }
            }
        }
        req[member][field] = max_req + 1;
        this->req_info[member][field] = req_info;
        return max_req + 1;
    }
    return 0;
}

template <typename T, typename ReqT>
const ReqT &SyncDataStore2<T, ReqT>::getReqInfo(const std::u8string &member,
                                                const std::u8string &field) {
    return req_info[member][field];
}


template <typename T, typename ReqT>
std::optional<T> SyncDataStore2<T, ReqT>::getRecv(const FieldBase &base) {
    return getRecv(base.member_, base.field_);
}
template <typename T, typename ReqT>
std::optional<T> SyncDataStore2<T, ReqT>::getRecv(const std::u8string &from,
                                                  const std::u8string &name) {
    std::lock_guard lock(mtx);
    if (from == self_member_name) {
        auto it = data_send.find(name);
        if (it != data_send.end()) {
            return it->second;
        }
    }
    // addReq(from, name);
    auto s_it = data_recv.find(from);
    if (s_it != data_recv.end()) {
        auto it = s_it->second.find(name);
        if (it != s_it->second.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}
template <typename T, typename ReqT>
std::optional<Dict<T>> SyncDataStore2<T, ReqT>::getRecvRecurse(
    const FieldBase &base,
    const std::function<void(const std::u8string &)> &cb) {
    return getRecvRecurse(base.member_, base.field_, cb);
}
template <typename T, typename ReqT>
std::optional<Dict<T>> SyncDataStore2<T, ReqT>::getRecvRecurse(
    const std::u8string &member, const std::u8string &field,
    const std::function<void(const std::u8string &)> &cb) {
    std::lock_guard lock(mtx);
    // addReq(member, field);
    auto s_it = data_recv.find(member);
    if (s_it != data_recv.end()) {
        Dict<T> d;
        bool found = false;
        for (const auto &it : s_it->second) {
            if (it.first.starts_with(field + field_separator)) {
                d[Encoding::decode(it.first.substr(field.size() + 1))] =
                    it.second;
                // addReq(member, it.first);
                found = true;
                if (cb) {
                    cb(it.first);
                }
            }
        }
        if (found) {
            return d;
        }
    }
    return std::nullopt;
}
template <typename T, typename ReqT>
bool SyncDataStore2<T, ReqT>::unsetRecv(const FieldBase &base) {
    return unsetRecv(base.member_, base.field_);
}
template <typename T, typename ReqT>
bool SyncDataStore2<T, ReqT>::unsetRecv(const std::u8string &from,
                                        const std::u8string &name) {
    std::lock_guard lock(mtx);
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
    if (!isSelf(from) && req[from][name] > 0) {
        req[from].erase(name);
        return true;
    }
    return false;
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::clearRecv(const FieldBase &base) {
    clearRecv(base.member_, base.field_);
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::clearRecv(const std::u8string &from,
                                        const std::u8string &name) {
    std::lock_guard lock(mtx);
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
    return;
}
template <typename T, typename ReqT>
std::pair<std::u8string, std::u8string>
SyncDataStore2<T, ReqT>::getReq(unsigned int req_id,
                                const std::u8string &sub_field) {
    std::lock_guard lock(mtx);
    for (const auto &r : req) {
        for (const auto &r2 : r.second) {
            if (r2.second == req_id) {
                if (!sub_field.empty() && sub_field[0] != '.') {
                    return std::make_pair(r.first,
                                          r2.first + u8'.' + sub_field);
                } else {
                    return std::make_pair(r.first, r2.first + sub_field);
                }
            }
        }
    }
    return std::make_pair(u8"", u8"");
}

template <typename T, typename ReqT>
std::unordered_map<std::u8string, T>
SyncDataStore2<T, ReqT>::transferSend(bool is_first) {
    std::lock_guard lock(mtx);
    std::unordered_map<std::u8string, T> send_changed;
    auto &recv_self = data_recv[self_member_name];
    for (auto &[name, data] : data_send) {
        auto r_it = recv_self.find(name);
        if (r_it == recv_self.end() || shouldSend(r_it->second, data)) {
            send_changed[name] = data;
            recv_self[name] = std::move(data);
        }
    }
    data_send.clear();
    if (is_first) {
        return data_send_prev = recv_self;
    } else {
        data_send_prev = send_changed;
        return send_changed;
    }
}
template <typename T, typename ReqT>
std::unordered_map<std::u8string, T>
SyncDataStore2<T, ReqT>::getSendPrev(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        return std::unordered_map<std::u8string, T>{};
    } else {
        return data_send_prev;
    }
}

template <typename T, typename ReqT>
std::unordered_map<std::u8string,
                   std::unordered_map<std::u8string, unsigned int>>
SyncDataStore2<T, ReqT>::transferReq() {
    std::lock_guard lock(mtx);
    // if (is_first) {
    // req_send.clear();
    return req;
    // } else {
    //     return std::move(req_send);
    // }
}

// ライブラリ外からは参照できないが、testのためにexportしている
template class WEBCFACE_DLL_INSTANCE_DEF
    SyncDataStore2<std::string, int>; // test用
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<ValueData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<TextData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<FuncData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<ViewData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<RobotModelData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<Canvas3DData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF SyncDataStore2<Canvas2DData, int>;
template class WEBCFACE_DLL_INSTANCE_DEF
    SyncDataStore2<ImageData, Common::ImageReq>;
} // namespace Internal
WEBCFACE_NS_END
