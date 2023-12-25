#include "data_store2.h"

namespace WEBCFACE_NS::Internal {
template <typename T>
void SyncDataStore2<T>::setSend(const std::string &name, const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    data_recv[self_member_name][name] = data; // 送信後に自分の値を参照する用
}

template <typename T>
void SyncDataStore2<T>::setRecv(const std::string &from,
                                const std::string &name, const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T>
std::vector<std::string> SyncDataStore2<T>::getMembers() {
    std::lock_guard lock(mtx);
    std::vector<std::string> k;
    for (const auto &r : entry) {
        k.push_back(r.first);
    }
    return k;
}
template <typename T>
std::vector<std::string> SyncDataStore2<T>::getEntry(const std::string &name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return std::vector<std::string>{};
    }
}
template <typename T>
void SyncDataStore2<T>::setEntry(const std::string &from) {
    std::lock_guard lock(mtx);
    entry.emplace(std::make_pair(from, std::vector<std::string>{}));
}
template <typename T>
void SyncDataStore2<T>::setEntry(const std::string &from,
                                 const std::string &e) {
    std::lock_guard lock(mtx);
    entry[from].push_back(e);
}

template <typename T>
unsigned int SyncDataStore2<T>::addReq(const std::string &member,
                                       const std::string &field) {
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

template <typename T>
std::optional<T> SyncDataStore2<T>::getRecv(const std::string &from,
                                            const std::string &name) {
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
template <typename T>
std::optional<Dict<T>> SyncDataStore2<T>::getRecvRecurse(
    const std::string &member, const std::string &field,
    const std::function<void(const std::string &)> &cb) {
    std::lock_guard lock(mtx);
    // addReq(member, field);
    auto s_it = data_recv.find(member);
    if (s_it != data_recv.end()) {
        Dict<T> d;
        bool found = false;
        for (const auto &it : s_it->second) {
            if (it.first.starts_with(field + ".")) {
                d[it.first.substr(field.size() + 1)] = it.second;
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
template <typename T>
bool SyncDataStore2<T>::unsetRecv(const std::string &from,
                                  const std::string &name) {
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
template <typename T>
std::pair<std::string, std::string>
SyncDataStore2<T>::getReq(unsigned int req_id, const std::string &sub_field) {
    std::lock_guard lock(mtx);
    for (const auto &r : req) {
        for (const auto &r2 : r.second) {
            if (r2.second == req_id) {
                if (!sub_field.empty() && sub_field[0] != '.') {
                    return std::make_pair(r.first, r2.first + "." + sub_field);
                } else {
                    return std::make_pair(r.first, r2.first + sub_field);
                }
            }
        }
    }
    return std::make_pair("", "");
}

template <typename T>
std::unordered_map<std::string, T>
SyncDataStore2<T>::transferSend(bool is_first) {
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
SyncDataStore2<T>::getSendPrev(bool is_first) {
    std::lock_guard lock(mtx);
    if (is_first) {
        return std::unordered_map<std::string, T>{};
    } else {
        return data_send_prev;
    }
}
template <typename T>
std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>>
SyncDataStore2<T>::transferReq() {
    std::lock_guard lock(mtx);
    // if (is_first) {
    // req_send.clear();
    return req;
    // } else {
    //     return std::move(req_send);
    // }
}


template class WEBCFACE_DLL SyncDataStore2<std::string>; // test用
template class WEBCFACE_DLL SyncDataStore2<std::shared_ptr<VectorOpt<double>>>;
template class WEBCFACE_DLL SyncDataStore2<std::shared_ptr<std::string>>;
template class WEBCFACE_DLL SyncDataStore2<std::shared_ptr<FuncInfo>>;
template class WEBCFACE_DLL
    SyncDataStore2<std::shared_ptr<std::vector<Common::ViewComponentBase>>>;

} // namespace WEBCFACE_NS::Internal
