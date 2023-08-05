#include <webcface/webcface.h>
#include <cassert>
#include <type_traits>

namespace WebCFace {
template <typename T>
void ClientData::SyncDataStore<T>::setSend(const std::string &name, const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    data_recv[""][name] = data; // 送信後に自分の値を参照する用
}
template <typename T>
void ClientData::SyncDataStore<T>::setRecv(const std::string &from, const std::string &name,
                               const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T>
std::vector<std::string> ClientData::SyncDataStore<T>::getMembers() {
    std::lock_guard lock(mtx);
    std::vector<std::string> k;
    for (const auto &r : entry) {
        k.push_back(r.first);
    }
    return k;
}
template <typename T>
std::vector<std::string> ClientData::SyncDataStore<T>::getEntry(const std::string &name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return std::vector<std::string>{};
    }
}
template <typename T>
void ClientData::SyncDataStore<T>::setEntry(const std::string &from) {
    std::lock_guard lock(mtx);
    entry.emplace(std::make_pair(from, std::vector<std::string>{}));
}
template <typename T>
void ClientData::SyncDataStore<T>::setEntry(const std::string &from, const std::string &e) {
    std::lock_guard lock(mtx);
    entry[from].push_back(e);
}
template <typename T>
std::optional<T> ClientData::SyncDataStore<T>::getRecv(const std::string &from,
                                           const std::string &name) {
    std::lock_guard lock(mtx);
    if (from != "" && (!req.count(from) || !req.at(from).count(name))) {
        req[from][name] = true;
        req_send[from][name] = true;
    }
    auto s_it = data_recv.find(from);
    if (s_it != data_recv.end()) {
        auto it = s_it->second.find(name);
        if (it != s_it->second.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}
template <typename T>
void ClientData::SyncDataStore<T>::unsetRecv(const std::string &from,
                                 const std::string &name) {
    std::lock_guard lock(mtx);
    if (from != "" && (req.count(from) && req.at(from).count(name))) {
        req.at(from).erase(name);
        req_send[from][name] = false;
    }
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
}
template <typename T>
std::unordered_map<std::string, T> ClientData::SyncDataStore<T>::transferSend() {
    std::lock_guard lock(mtx);
    return std::move(data_send);
}
template <typename T>
std::unordered_map<std::string, std::unordered_map<std::string, bool>>
ClientData::SyncDataStore<T>::transferReq() {
    std::lock_guard lock(mtx);
    return std::move(req_send);
}

template class SyncDataStore<double>;
template class SyncDataStore<std::string>;
template class SyncDataStore<FuncInfo>;

} // namespace WebCFace