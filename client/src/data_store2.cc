#include "webcface/internal/data_store2.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/field.h"
#include "webcface/internal/func_internal.h"
#include "webcface/image_frame.h"
#include "webcface/robot_link.h"
#include "webcface/internal/robot_link_internal.h"
#include "webcface/internal/log_history.h"

WEBCFACE_NS_BEGIN
namespace internal {

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
SyncDataStore2<T, ResendCondition, ReqT, EntryT>::SyncDataStore2(
    const SharedString &name)
    : self_member_name(name) {}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
bool SyncDataStore2<T, ResendCondition, ReqT, EntryT>::isSelf(
    const SharedString &member) const {
    return member == self_member_name;
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::setSend(
    const FieldBase &base, const std::shared_ptr<T> &data) {
    setSend(base.field_, data);
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::setSend(
    const SharedString &name, const std::shared_ptr<T> &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    // auto &recv_self = data_recv[self_member_name];
    // recv_self[name] = data; // 送信後に自分の値を参照する用
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::setRecv(
    const FieldBase &base, const std::shared_ptr<T> &data) {
    setRecv(base.member_, base.field_, data);
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::setRecv(
    const SharedString &from, const SharedString &name,
    const std::shared_ptr<T> &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
EntryT *SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getEntryP(
    const SharedString &member, const SharedString &name) {
    std::lock_guard lock(mtx);
    if (entry[member].count(name)) {
        return &entry[member][name];
    } else {
        return nullptr;
    }
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
StrMap1<EntryT> &SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getEntry(
    const SharedString &name) {
    std::lock_guard lock(mtx);
    return entry[name];
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::initMember(
    const SharedString &from) {
    std::lock_guard lock(mtx);
    entry[from].clear();
    data_recv[from].clear();
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::setEntry(
    const SharedString &from, const SharedString &e, EntryT e_data) {
    std::lock_guard lock(mtx);
    entry[from][e] = std::move(e_data);
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::setEntry(
    const FieldBase &base, EntryT e_data) {
    setEntry(base.member_, base.field_, std::move(e_data));
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
unsigned int SyncDataStore2<T, ResendCondition, ReqT, EntryT>::addReq(
    const SharedString &member, const SharedString &field) {
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
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
unsigned int SyncDataStore2<T, ResendCondition, ReqT, EntryT>::addReq(
    const SharedString &member, const SharedString &field, const ReqT &info) {
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

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
const ReqT &SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getReqInfo(
    const SharedString &member, const SharedString &field) {
    return req_info[member][field];
}


template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
std::shared_ptr<T> SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getRecv(
    const FieldBase &base) {
    return getRecv(base.member_, base.field_);
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
std::shared_ptr<T> SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getRecv(
    const SharedString &from, const SharedString &name) {
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
    return nullptr;
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
bool SyncDataStore2<T, ResendCondition, ReqT, EntryT>::unsetRecv(
    const FieldBase &base) {
    return unsetRecv(base.member_, base.field_);
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
bool SyncDataStore2<T, ResendCondition, ReqT, EntryT>::unsetRecv(
    const SharedString &from, const SharedString &name) {
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
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::clearRecv(
    const FieldBase &base) {
    clearRecv(base.member_, base.field_);
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
void SyncDataStore2<T, ResendCondition, ReqT, EntryT>::clearRecv(
    const SharedString &from, const SharedString &name) {
    std::lock_guard lock(mtx);
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
    return;
}
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
std::pair<SharedString, SharedString>
SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getReq(
    unsigned int req_id, const SharedString &sub_field) {
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

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
StrMap1<std::shared_ptr<T>>
SyncDataStore2<T, ResendCondition, ReqT, EntryT>::transferSend(bool is_first) {
    std::lock_guard lock(mtx);
    StrMap1<std::shared_ptr<T>> send_changed;
    auto &recv_self = data_recv[self_member_name];
    for (auto &[name, data] : data_send) {
        auto r_it = recv_self.find(name);
        if (r_it == recv_self.end() ||
            ResendCondition::shouldResend(r_it->second, data)) {
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
template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
StrMap1<std::shared_ptr<T>>
SyncDataStore2<T, ResendCondition, ReqT, EntryT>::getSendPrev(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        return StrMap1<std::shared_ptr<T>>{};
    } else {
        return data_send_prev;
    }
}

template <typename T, typename ResendCondition, typename ReqT, typename EntryT>
StrMap2<unsigned int>
SyncDataStore2<T, ResendCondition, ReqT, EntryT>::transferReq() {
    std::lock_guard lock(mtx);
    // if (is_first) {
    // req_send.clear();
    return req;
    // } else {
    //     return std::move(req_send);
    // }
}

template class SyncDataStore2<const std::string, ResendOnChange>; // test用
template class SyncDataStore2<const std::vector<double>, ResendOnChange, int,
                              message::ValueShape>;
template class SyncDataStore2<const ValAdaptor, ResendOnChange>;
template class SyncDataStore2<FuncInfo, ResendNever>;
template class SyncDataStore2<
    const std::vector<std::shared_ptr<internal::RobotLinkData>>>;
template class SyncDataStore2<const ImageFrame, ResendAlways,
                              message::ImageReq>;
template class SyncDataStore2<const message::ViewData>;
template class SyncDataStore2<const message::Canvas3DData>;
template class SyncDataStore2<const message::Canvas2DData>;
template class SyncDataStore2<LogHistory>;
} // namespace internal
WEBCFACE_NS_END
