#include <webcface/client_data.h>

namespace WebCFace {
template <typename T>
void ClientData::SyncDataStore<T>::setSend(const std::string &name,
                                           const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    data_recv[self_member_name][name] = data; // 送信後に自分の値を参照する用
}
template <typename T>
void ClientData::SyncDataStore<T>::setRecv(const std::string &from,
                                           const std::string &name,
                                           const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

void ClientData::LogStore::addRecv(const std::string &member,
                                   const LogLine &log) {
    std::lock_guard lock(mtx);
    data_recv[member].push_back(log);
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
std::vector<std::string>
ClientData::SyncDataStore<T>::getEntry(const std::string &name) {
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
void ClientData::SyncDataStore<T>::setEntry(const std::string &from,
                                            const std::string &e) {
    std::lock_guard lock(mtx);
    entry[from].push_back(e);
}

template <typename T>
std::optional<T>
ClientData::SyncDataStore<T>::getRecv(const std::string &from,
                                      const std::string &name) {
    std::lock_guard lock(mtx);
    if (!isSelf(from) && (!req.count(from) || !req.at(from).count(name) ||
                          req[from][name] == false)) {
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
std::optional<std::vector<LogLine>>
ClientData::LogStore::getRecv(const std::string &member) {
    std::lock_guard lock(mtx);
    if (!isSelf(member) && (!req.count(member) || req[member] == false)) {
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
void ClientData::SyncDataStore<T>::unsetRecv(const std::string &from,
                                             const std::string &name) {
    std::lock_guard lock(mtx);
    if (!isSelf(from) && (req.count(from) && req.at(from).count(name) &&
                          req[from][name] == true)) {
        req[from].erase(name);
        req_send[from][name] = false;
    }
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
}
template <typename T>
std::unordered_map<std::string, T>
ClientData::SyncDataStore<T>::transferSend() {
    std::lock_guard lock(mtx);
    return std::move(data_send);
}
template <typename T>
std::unordered_map<std::string, std::unordered_map<std::string, bool>>
ClientData::SyncDataStore<T>::transferReq() {
    std::lock_guard lock(mtx);
    return std::move(req_send);
}
std::unordered_map<std::string, bool> ClientData::LogStore::transferReq() {
    std::lock_guard lock(mtx);
    return std::move(req_send);
}

template class ClientData::SyncDataStore<double>;
template class ClientData::SyncDataStore<std::string>;
template class ClientData::SyncDataStore<FuncInfo>;


AsyncFuncResult &
ClientData::FuncResultStore::addResult(const std::string &caller,
                                       const FieldBase &base) {
    std::lock_guard lock(mtx);
    int caller_id = results.size();
    results.push_back(AsyncFuncResult{caller_id, caller, base});
    return results.back();
}
AsyncFuncResult &ClientData::FuncResultStore::getResult(int caller_id) {
    std::lock_guard lock(mtx);
    return results.at(caller_id);
}


} // namespace WebCFace