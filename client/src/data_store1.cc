#include "webcface/internal/data_store1.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename T>
SyncDataStore1<T>::SyncDataStore1(const SharedString &name)
    : self_member_name(name) {}

template <typename T>
bool SyncDataStore1<T>::isSelf(const SharedString &member) const {
    return member == self_member_name;
}

template <typename T>
void SyncDataStore1<T>::setRecv(const SharedString &member, const T &data) {
    std::lock_guard lock(mtx);
    data_recv[member] = data;
}

template <typename T>
bool SyncDataStore1<T>::addReq(const SharedString &member) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && req[member] == false) {
        req[member] = true;
        return true;
    }
    return false;
}
template <typename T>
bool SyncDataStore1<T>::clearReq(const SharedString &member) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && req[member] == true) {
        req[member] = false;
        return true;
    }
    return false;
}

template <typename T>
std::optional<T> SyncDataStore1<T>::getRecv(const SharedString &member) {
    std::lock_guard lock(mtx);
    auto s_it = data_recv.find(member);
    if (s_it != data_recv.end()) {
        return s_it->second;
    }
    return std::nullopt;
}
template <typename T>
StrMap1<bool> SyncDataStore1<T>::transferReq() {
    std::lock_guard lock(mtx);
    // if (is_first) {
    req_send.clear();
    return req;
    // } else {
    //     return std::move(req_send);
    // }
}

template class SyncDataStore1<std::string>; // test用
template class SyncDataStore1<std::shared_ptr<std::vector<LogLineData>>>;
template class SyncDataStore1<std::chrono::system_clock::time_point>;

} // namespace internal
WEBCFACE_NS_END
