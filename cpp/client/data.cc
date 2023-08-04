#include <webcface/webcface.h>
#include <cassert>
#include <type_traits>

namespace WebCFace {
template <typename T>
void SyncDataStore<T>::setSend(const std::string &name, const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    data_recv[""][name] = data; // 送信後に自分の値を参照する用
}
template <typename T>
void SyncDataStore<T>::setRecv(const std::string &from, const std::string &name,
                               const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T>
std::vector<std::string> SyncDataStore<T>::getMembers() {
    std::lock_guard lock(mtx);
    std::vector<std::string> k;
    for (const auto &r : entry) {
        k.push_back(r.first);
    }
    return k;
}
template <typename T>
std::vector<std::string> SyncDataStore<T>::getEntry(const std::string &name) {
    std::lock_guard lock(mtx);
    auto e = entry.find(name);
    if (e != entry.end()) {
        return e->second;
    } else {
        return std::vector<std::string>{};
    }
}
template <typename T>
void SyncDataStore<T>::setEntry(const std::string &from,
                                const std::vector<std::string> &e) {
    std::lock_guard lock(mtx);
    entry[from] = e;
}
template <typename T>
std::optional<T> SyncDataStore<T>::getRecv(const std::string &from,
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
void SyncDataStore<T>::unsetRecv(const std::string &from,
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
std::unordered_map<std::string, T> SyncDataStore<T>::transferSend() {
    std::lock_guard lock(mtx);
    return std::move(data_send);
}
template <typename T>
std::unordered_map<std::string, std::unordered_map<std::string, bool>>
SyncDataStore<T>::transferReq() {
    std::lock_guard lock(mtx);
    return std::move(req_send);
}

template <typename T>
SyncData<T>::SyncData(Client *cli, const std::string &member,
                      const std::string &name)
    : cli(cli), member_(member), name_(name) {
    if constexpr (std::is_same_v<T, Value::DataType>) {
        store = &cli->value_store;
    } else if constexpr (std::is_same_v<T, Text::DataType>) {
        store = &cli->text_store;
    } else if constexpr (std::is_same_v<T, Func::DataType>) {
        store = &cli->func_store;
    } else {
        store = nullptr;
    }
}

template <typename T>
Member SyncData<T>::member() const {
    return cli->member(member_);
}

template <typename T>
SyncData<T> &SyncData<T>::set(const T &data) {
    assert(member_ == "" && "Cannot set data to member other than self");
    store->setSend(name_, data);
    return *this;
}

template <typename T>
std::optional<T> SyncData<T>::try_get() const {
    return store->getRecv(member_, name_);
}
template <typename T>
T SyncData<T>::get() const {
    auto v = try_get();
    if (v) {
        return *v;
    } else {
        return T{};
    }
}

// インスタンス化
#define INSTANTIATE(T)                                                         \
    template class SyncData<T>;                                                \
    template class SyncDataStore<T>;

INSTANTIATE(Value::DataType);
INSTANTIATE(Text::DataType);
INSTANTIATE(Func::DataType);

} // namespace WebCFace