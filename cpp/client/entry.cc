#include "entry.h"
#include <webcface/webcface.h>
#include <iostream>
#include <cassert>

namespace WebCFace {
template <typename T>
void SyncDataStore<T>::set_send(const std::string &name, const T &data) {
    std::lock_guard lock(mtx);
    data_send[name] = data;
    data_recv[""][name] = data; // 送信後に自分の値を参照する用
}
template <typename T>
void SyncDataStore<T>::set_recv(const std::string &from,
                                const std::string &name, const T &data) {
    std::lock_guard lock(mtx);
    data_recv[from][name] = data;
}

template <typename T>
std::optional<T> SyncDataStore<T>::try_get_recv(const std::string &from,
                                                const std::string &name) {
    std::lock_guard lock(mtx);
    auto p = std::make_pair(from, name);
    if (from != "" && subsc.count(p) == 0) {
        subsc.insert(p);
        subsc_next.insert(p);
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
std::unordered_map<std::string, T> SyncDataStore<T>::transfer_send() {
    std::lock_guard lock(mtx);
    return std::move(data_send);
}
template <typename T>
std::set<std::pair<std::string, std::string>>
SyncDataStore<T>::transfer_subsc() {
    std::lock_guard lock(mtx);
    return std::move(subsc_next);
}

template <typename T>
SyncData<T> &SyncData<T>::set(const T &data) {
    assert(from == "");
    store->set_send(name, data);
    return *this;
}

template <typename T>
std::optional<T> SyncData<T>::try_get() const {
    return store->try_get_recv(from, name);
}
template <typename T>
T SyncData<T>::get() const {
    auto v = try_get();
    if (v) {
        return *v;
    } else {
        return 0;
    }
}

// インスタンス化
#define instantiate(T)                                                         \
    template class SyncData<T::DataType>;                                      \
    template class SyncDataStore<T::DataType>;

instantiate(Value);

} // namespace WebCFace