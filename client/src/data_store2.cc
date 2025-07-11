#include "webcface/internal/data_store2.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/field.h"
#include <type_traits>
#include "webcface/internal/func_internal.h"
#include "webcface/image_frame.h"
#include "webcface/robot_link.h"
#include "webcface/internal/robot_link_internal.h"
#include "webcface/log.h"

WEBCFACE_NS_BEGIN
namespace internal {
/*!
 * \brief setSend時にこれを実際に送信すべきかどうか
 * \private
 */
template <typename T>
static bool shouldSend(const T &prev, const T &current) {
    if constexpr (std::is_same_v<T, std::shared_ptr<ValueData>> ||
                  std::is_same_v<T, std::shared_ptr<TextData>>) {
        return *prev != *current;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return prev != current;
    } else if constexpr (std::is_same_v<T, std::shared_ptr<FuncData>>) {
        // Funcは内容が変更されても2回目以降送信しない
        return false;
    } else {
        return true;
    }
}

template <typename T, typename ReqT>
SyncDataStore2<T, ReqT>::SyncDataStore2(const SharedString &name)
    : self_member_name(name) {}

template <typename T, typename ReqT>
bool SyncDataStore2<T, ReqT>::isSelf(const SharedString &member) const {
    return member == self_member_name;
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setSend(const FieldBase &base, const T &data) {
    setSend(base.field_, data);
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setSend(const SharedString &name, const T &data) {
    std::lock_guard lock(mtx);
    data_send.emplace_back(name, data);
    // auto &recv_self = data_recv[self_member_name];
    // recv_self[name] = data; // 送信後に自分の値を参照する用
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setRecv(const FieldBase &base, const T &data) {
    setRecv(base.member_, base.field_, data);
}

template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setRecv(const SharedString &from,
                                      const SharedString &name, const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T, typename ReqT>
StrSet1 SyncDataStore2<T, ReqT>::getEntry(const FieldBase &base) {
    return getEntry(base.member_);
}
template <typename T, typename ReqT>
StrSet1 SyncDataStore2<T, ReqT>::getEntry(const SharedString &name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return StrSet1{};
    }
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::initMember(const SharedString &from) {
    std::lock_guard lock(mtx);
    entry[from].clear();
    data_recv[from].clear();
}
template <typename T, typename ReqT>
void SyncDataStore2<T, ReqT>::setEntry(const SharedString &from,
                                       const SharedString &e) {
    std::lock_guard lock(mtx);
    entry[from].emplace(e);
}

template <typename T, typename ReqT>
unsigned int SyncDataStore2<T, ReqT>::addReq(const SharedString &member,
                                             const SharedString &field) {
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
unsigned int SyncDataStore2<T, ReqT>::addReq(const SharedString &member,
                                             const SharedString &field,
                                             const ReqT &info) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) &&
        (req[member][field] == 0 || this->req_info[member][field] != info)) {
        unsigned int max_req = 0;
        for (const auto &r : req) {
            for (const auto &r2 : r.second) {
                if (r2.second > max_req) {
                    max_req = r2.second;
                }
            }
        }
        req[member][field] = max_req + 1;
        this->req_info[member][field] = info;
        return max_req + 1;
    }
    return 0;
}

template <typename T, typename ReqT>
const ReqT &SyncDataStore2<T, ReqT>::getReqInfo(const SharedString &member,
                                                const SharedString &field) {
    return req_info[member][field];
}


template <typename T, typename ReqT>
std::optional<T> SyncDataStore2<T, ReqT>::getRecv(const FieldBase &base) {
    return getRecv(base.member_, base.field_);
}
template <typename T, typename ReqT>
std::optional<T> SyncDataStore2<T, ReqT>::getRecv(const SharedString &from,
                                                  const SharedString &name) {
    std::lock_guard lock(mtx);
    if (from == self_member_name) {
        // emplace_backしているので後ろが最新
        for(int i = static_cast<int>(data_send.size()) - 1; i >= 0; i--){
            if(data_send[i].first == name){
                return data_send[i].second;
            }
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
bool SyncDataStore2<T, ReqT>::unsetRecv(const FieldBase &base) {
    return unsetRecv(base.member_, base.field_);
}
template <typename T, typename ReqT>
bool SyncDataStore2<T, ReqT>::unsetRecv(const SharedString &from,
                                        const SharedString &name) {
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
void SyncDataStore2<T, ReqT>::clearRecv(const SharedString &from,
                                        const SharedString &name) {
    std::lock_guard lock(mtx);
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
    return;
}
template <typename T, typename ReqT>
std::pair<SharedString, SharedString>
SyncDataStore2<T, ReqT>::getReq(unsigned int req_id,
                                const SharedString &sub_field) {
    std::lock_guard lock(mtx);
    for (const auto &r : req) {
        for (const auto &r2 : r.second) {
            if (r2.second == req_id) {
                if (!sub_field.empty() &&
                    sub_field.u8String()[0] != field_separator) {
                    return std::make_pair(r.first, SharedString::fromU8String(
                                                       r2.first.u8String() +
                                                       field_separator +
                                                       sub_field.u8String()));
                } else {
                    return std::make_pair(r.first, SharedString::fromU8String(
                                                       r2.first.u8String() +
                                                       sub_field.u8String()));
                }
            }
        }
    }
    return std::make_pair(nullptr, nullptr);
}

template <typename T, typename ReqT>
StrMap1<T> SyncDataStore2<T, ReqT>::transferSend(bool is_first) {
    std::lock_guard lock(mtx);
    StrMap1<T> send_changed;
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
StrMap1<T> SyncDataStore2<T, ReqT>::getSendPrev(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        return StrMap1<T>{};
    } else {
        return data_send_prev;
    }
}

template <typename T, typename ReqT>
StrMap2<unsigned int> SyncDataStore2<T, ReqT>::transferReq() {
    std::lock_guard lock(mtx);
    // if (is_first) {
    // req_send.clear();
    return req;
    // } else {
    //     return std::move(req_send);
    // }
}

template class SyncDataStore2<std::string, int>; // test用
template class SyncDataStore2<std::shared_ptr<ValueData>, int>;
template class SyncDataStore2<std::shared_ptr<TextData>, int>;
template class SyncDataStore2<std::shared_ptr<FuncData>, int>;
template class SyncDataStore2<std::shared_ptr<message::ViewData>, int>;
template class SyncDataStore2<std::shared_ptr<RobotModelData>, int>;
template class SyncDataStore2<std::shared_ptr<message::Canvas3DData>, int>;
template class SyncDataStore2<std::shared_ptr<message::Canvas2DData>, int>;
template class SyncDataStore2<ImageData, message::ImageReq>;
template class SyncDataStore2<std::shared_ptr<LogData>, int>;
} // namespace internal
WEBCFACE_NS_END
