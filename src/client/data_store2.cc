#include "data_store2.h"
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
void SyncDataStore2<T, ReqT>::setSend(MemberNameRef name, const T &data) {
    std::lock_guard lock(mtx);
    auto &recv_self = data_recv[self_member_name];
    if (!recv_self.count(name) || shouldSend(recv_self[name], data)) {
        data_send[name] = data;
    }
    recv_self[name] = data; // 送信後に自分の値を参照する用
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setRecv(MemberNameRef from, FieldNameRef name,
                                      const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T, typename ReqT>
std::vector<FieldNameRef>
SyncDataStore2<T, ReqT>::getEntry(MemberNameRef name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return std::vector<FieldNameRef>{};
    }
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setEntry(MemberNameRef from, FieldNameRef e) {
    std::lock_guard lock(mtx);
    entry[from].push_back(e);
}

template <typename T, typename ReqT>
unsigned int SyncDataStore2<T, ReqT>::addReq(MemberNameRef member,
                                             FieldNameRef field) {
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
unsigned int SyncDataStore2<T, ReqT>::addReq(MemberNameRef member,
                                             FieldNameRef field,
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
const ReqT &SyncDataStore2<T, ReqT>::getReqInfo(MemberNameRef member,
                                                FieldNameRef field) {
    return req_info[member][field];
}


template <typename T, typename ReqT>
std::optional<T> SyncDataStore2<T, ReqT>::getRecv(MemberNameRef from,
                                                  FieldNameRef name) {
    std::lock_guard lock(mtx);
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
    MemberNameRef member, FieldNameRef field,
    const std::function<void(FieldNameRef)> &cb) {
    std::lock_guard lock(mtx);
    // addReq(member, field);
    auto s_it = data_recv.find(member);
    if (s_it != data_recv.end()) {
        Dict<T> d;
        bool found = false;
        for (const auto &it : s_it->second) {
            if (Encoding::getNameU8(it.first).starts_with(
                    std::u8string(Encoding::getNameU8(field)) + u8".")) {
#pragma message("still using string for Dict index")
                d[Encoding::getName(Encoding::getNameU8(it.first).substr(
                    Encoding::getNameU8(field).size() + 1))] = it.second;
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
bool SyncDataStore2<T, ReqT>::unsetRecv(MemberNameRef from, FieldNameRef name) {
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
void SyncDataStore2<T, ReqT>::clearRecv(MemberNameRef from, FieldNameRef name) {
    std::lock_guard lock(mtx);
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
    return;
}
template <typename T, typename ReqT>
std::pair<MemberNameRef, std::u8string>
SyncDataStore2<T, ReqT>::getReq(unsigned int req_id,
                                const std::u8string &sub_field) {
    std::lock_guard lock(mtx);
    for (const auto &r : req) {
        for (const auto &r2 : r.second) {
            if (r2.second == req_id) {
                if (!sub_field.empty() && !sub_field.starts_with(u8'.')) {
                    return std::make_pair(
                        r.first, std::u8string(Encoding::getNameU8(r2.first)) +
                                     u8"." + sub_field);
                } else {
                    return std::make_pair(
                        r.first, std::u8string(Encoding::getNameU8(r2.first)) +
                                     sub_field);
                }
            }
        }
    }
    return std::make_pair(nullptr, u8"");
}

template <typename T, typename ReqT>
std::unordered_map<MemberNameRef, T>
SyncDataStore2<T, ReqT>::transferSend(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        data_send.clear();
        return data_send_prev = data_recv[self_member_name];
    } else {
        data_send_prev = data_send;
        return std::move(data_send);
    }
}
template <typename T, typename ReqT>
std::unordered_map<MemberNameRef, T>
SyncDataStore2<T, ReqT>::getSendPrev(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        return std::unordered_map<MemberNameRef, T>{};
    } else {
        return data_send_prev;
    }
}

template <typename T, typename ReqT>
std::unordered_map<MemberNameRef,
                   std::unordered_map<FieldNameRef, unsigned int>>
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
template class WEBCFACE_DLL SyncDataStore2<std::string, int>; // test用
template class WEBCFACE_DLL SyncDataStore2<ValueData, int>;
template class WEBCFACE_DLL SyncDataStore2<TextData, int>;
template class WEBCFACE_DLL SyncDataStore2<FuncData, int>;
template class WEBCFACE_DLL SyncDataStore2<ViewData, int>;
template class WEBCFACE_DLL SyncDataStore2<RobotModelData, int>;
template class WEBCFACE_DLL SyncDataStore2<Canvas3DData, int>;
template class WEBCFACE_DLL SyncDataStore2<Canvas2DData, int>;
template class WEBCFACE_DLL SyncDataStore2<ImageData, Common::ImageReq>;
} // namespace Internal
WEBCFACE_NS_END
