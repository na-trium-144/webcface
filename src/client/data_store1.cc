#include "data_store1.h"

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename T>
void SyncDataStore1<T>::setRecv(MemberNameRef member, const T &data) {
    std::lock_guard lock(mtx);
    data_recv[member] = data;
}

template <typename T>
bool SyncDataStore1<T>::addReq(MemberNameRef member) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && req[member] == false) {
        req[member] = true;
        return true;
    }
    return false;
}
template <typename T>
bool SyncDataStore1<T>::clearReq(MemberNameRef member) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && req[member] == true) {
        req[member] = false;
        return true;
    }
    return false;
}

template <typename T>
std::optional<T> SyncDataStore1<T>::getRecv(MemberNameRef member) {
    std::lock_guard lock(mtx);
    auto s_it = data_recv.find(member);
    if (s_it != data_recv.end()) {
        return s_it->second;
    }
    return std::nullopt;
}
template <typename T>
std::unordered_map<MemberNameRef, bool> SyncDataStore1<T>::transferReq() {
    std::lock_guard lock(mtx);
    // if (is_first) {
    req_send.clear();
    return req;
    // } else {
    //     return std::move(req_send);
    // }
}

template class WEBCFACE_DLL SyncDataStore1<std::string>; // test用
template class WEBCFACE_DLL
    SyncDataStore1<std::shared_ptr<std::vector<Common::LogLine>>>;
template class WEBCFACE_DLL
    SyncDataStore1<std::chrono::system_clock::time_point>;

} // namespace Internal
WEBCFACE_NS_END
