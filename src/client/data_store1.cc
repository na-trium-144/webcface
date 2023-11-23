#include "data_store1.h"

namespace webcface::Internal {
template <typename T>
void SyncDataStore1<T>::setRecv(const std::string &member, const T &data) {
    std::lock_guard lock(mtx);
    data_recv[member] = data;
}

template <typename T>
std::optional<T> SyncDataStore1<T>::getRecv(const std::string &member) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && req[member] == false) {
        req[member] = true;
        req_send[member] = true;
    }
    auto s_it = data_recv.find(member);
    if (s_it != data_recv.end()) {
        return s_it->second;
    }
    return std::nullopt;
}
template <typename T>
std::unordered_map<std::string, bool>
SyncDataStore1<T>::transferReq(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        req_send.clear();
        return req;
    } else {
        return std::move(req_send);
    }
}

template class WEBCFACE_DLL SyncDataStore1<std::string>; // test用
template class WEBCFACE_DLL SyncDataStore1<
    std::shared_ptr<std::vector<std::shared_ptr<Common::LogLine>>>>;
template class WEBCFACE_DLL
    SyncDataStore1<std::chrono::system_clock::time_point>;

} // namespace webcface::Internal