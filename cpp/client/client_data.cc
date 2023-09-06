#include <webcface/client_data.h>

namespace WebCFace {
template <typename T>
void ClientData::SyncDataStore2<T>::setSend(const std::string &name,
                                            const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    data_recv[self_member_name][name] = data; // 送信後に自分の値を参照する用
}
template <typename T>
void ClientData::SyncDataStore2<T>::setHidden(const std::string &name,
                                              bool is_hidden) {
    std::lock_guard lock(mtx);
    data_send_hidden[name] = is_hidden;
}
template <typename T>
bool ClientData::SyncDataStore2<T>::isHidden(const std::string &name) {
    std::lock_guard lock(mtx);
    auto h = data_send_hidden.find(name);
    return h != data_send_hidden.end() && h->second == true;
}

template <typename T>
void ClientData::SyncDataStore2<T>::setRecv(const std::string &from,
                                            const std::string &name,
                                            const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}
template <typename T>
void ClientData::SyncDataStore1<T>::setRecv(const std::string &member,
                                            const T &data) {
    std::lock_guard lock(mtx);
    data_recv[member] = data;
}
template <typename T>
template <typename U>
    requires std::same_as<T, std::vector<U>>
void ClientData::SyncDataStore1<T>::addRecv(const std::string &member,
                                            const U &data) {
    std::lock_guard lock(mtx);
    data_recv[member].push_back(data);
}

template <typename T>
std::vector<std::string> ClientData::SyncDataStore2<T>::getMembers() {
    std::lock_guard lock(mtx);
    std::vector<std::string> k;
    for (const auto &r : entry) {
        k.push_back(r.first);
    }
    return k;
}
template <typename T>
std::vector<std::string>
ClientData::SyncDataStore2<T>::getEntry(const std::string &name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return std::vector<std::string>{};
    }
}
template <typename T>
void ClientData::SyncDataStore2<T>::setEntry(const std::string &from) {
    std::lock_guard lock(mtx);
    entry.emplace(std::make_pair(from, std::vector<std::string>{}));
}
template <typename T>
void ClientData::SyncDataStore2<T>::setEntry(const std::string &from,
                                             const std::string &e) {
    std::lock_guard lock(mtx);
    entry[from].push_back(e);
}

template <typename T>
std::optional<T>
ClientData::SyncDataStore2<T>::getRecv(const std::string &from,
                                       const std::string &name) {
    std::lock_guard lock(mtx);
    if (!isSelf(from) && req[from][name] <= 0) {
        unsigned int max_req = 0;
        for (const auto &r : req) {
            for (const auto &r2 : r.second) {
                if (r2.second > max_req) {
                    max_req = r2.second;
                }
            }
        }
        req[from][name] = max_req + 1;
        req_send[from][name] = max_req + 1;
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
std::optional<T>
ClientData::SyncDataStore1<T>::getRecv(const std::string &member) {
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
void ClientData::SyncDataStore2<T>::unsetRecv(const std::string &from,
                                              const std::string &name) {
    std::lock_guard lock(mtx);
    if (!isSelf(from) && req[from][name] > 0) {
        req[from].erase(name);
        req_send[from][name] = 0;
    }
    if (data_recv.count(from) && data_recv.at(from).count(name)) {
        data_recv.at(from).erase(name);
    }
}
template <typename T>
std::pair<std::string, std::string>
ClientData::SyncDataStore2<T>::getReq(unsigned int req_id) {
    std::lock_guard lock(mtx);
    for (const auto &r : req) {
        for (const auto &r2 : r.second) {
            if (r2.second == req_id) {
                return std::make_pair(r.first, r2.first);
            }
        }
    }
    return std::make_pair("", "");
}

template <typename T>
std::unordered_map<std::string, T>
ClientData::SyncDataStore2<T>::transferSend(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        data_send.clear();
        return data_send_prev = data_recv[self_member_name];
    } else {
        data_send_prev = data_send;
        return std::move(data_send);
    }
}
template <typename T>
std::unordered_map<std::string, T>
ClientData::SyncDataStore2<T>::getSendPrev(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        return std::unordered_map<std::string, T>{};
    } else {
        return data_send_prev;
    }
}
template <typename T>
std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>>
ClientData::SyncDataStore2<T>::transferReq(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        req_send.clear();
        return req;
    } else {
        return std::move(req_send);
    }
}
template <typename T>
std::unordered_map<std::string, bool>
ClientData::SyncDataStore1<T>::transferReq(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        req_send.clear();
        return req;
    } else {
        return std::move(req_send);
    }
}

template class ClientData::SyncDataStore2<double>;
template class ClientData::SyncDataStore2<std::string>;
template class ClientData::SyncDataStore2<FuncInfo>;
template class ClientData::SyncDataStore2<std::vector<ViewComponentBase>>;
template class ClientData::SyncDataStore1<std::vector<LogLine>>;
template void
ClientData::SyncDataStore1<std::vector<LogLine>>::addRecv(const std::string &,
                                                          const LogLine &);
template class ClientData::SyncDataStore1<
    std::chrono::system_clock::time_point>;

AsyncFuncResult &
ClientData::FuncResultStore::addResult(const std::string &caller,
                                       const Field &base) {
    std::lock_guard lock(mtx);
    unsigned int caller_id = results.size();
    results.push_back(AsyncFuncResult{caller_id, caller, base});
    return results.back();
}
AsyncFuncResult &ClientData::FuncResultStore::getResult(int caller_id) {
    std::lock_guard lock(mtx);
    return results.at(caller_id);
}

std::string ClientData::getMemberNameFromId(unsigned int id) const {
    for (const auto &it : member_ids) {
        if (it.second == id) {
            return it.first;
        }
    }
    return "";
}
unsigned int ClientData::getMemberIdFromName(const std::string &name) const {
    auto it = member_ids.find(name);
    if (it != member_ids.end()) {
        return it->second;
    }
    return 0;
}
} // namespace WebCFace