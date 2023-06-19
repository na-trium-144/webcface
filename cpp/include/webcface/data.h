#pragma once
#include <string>
#include <memory>
#include <optional>

namespace WebCFace {

class Client;

template <typename T>
class SyncData {
  private:
    std::shared_ptr<SyncDataStore<T>> store;
    std::string from, name;

  public:
    using DataType = T;
    SyncData(std::shared_ptr<SyncDataStore<T>> store, const std::string &from,
             const std::string &name)
        : store(store), from(from), name(name) {}
    SyncData<T> &set(const T &data);
    SyncData<T> &operator=(const T &data) { return set(data); }
    T get() const;
    operator T() const { return get(); }
    std::optional<T> try_get() const;

    // 演算が可能な型に対しては実装できる
    template <typename U>
    auto &operator+=(const U &rhs) {
        return this->set(this->get() + rhs);
    }
    template <typename U>
    auto &operator-=(const U &rhs) {
        return this->set(this->get() - rhs);
    }
    template <typename U>
    auto &operator*=(const U &rhs) {
        return this->set(this->get() * rhs);
    }
    template <typename U>
    auto &operator/=(const U &rhs) {
        return this->set(this->get() / rhs);
    }
    template <typename U>
    auto &operator%=(const U &rhs) {
        return this->set(this->get() % rhs);
    }
    template <typename U>
    auto &operator<<=(const U &rhs) {
        return this->set(this->get() << rhs);
    }
    template <typename U>
    auto &operator>>=(const U &rhs) {
        return this->set(this->get() >> rhs);
    }
    template <typename U>
    auto &operator&=(const U &rhs) {
        return this->set(this->get() & rhs);
    }
    template <typename U>
    auto &operator|=(const U &rhs) {
        return this->set(this->get() | rhs);
    }
    template <typename U>
    auto &operator^=(const U &rhs) {
        return this->set(this->get() ^ rhs);
    }
};

using Value = SyncData<double>;
using Text = SyncData<std::string>;


inline auto &operator++(Value &s) { // ++s
    auto v = s.get();
    s.set(v + 1);
    return s;
}
inline auto operator++(Value &&ss) {
    auto s = ss;
    return ++s;
}
inline auto operator++(Value &s, int) { // s++
    auto v = s.get();
    s.set(v + 1);
    return v;
}
inline auto operator++(Value &&ss, int) {
    auto s = ss;
    return s++;
}
inline auto &operator--(Value &s) { // --s
    auto v = s.get();
    s.set(v - 1);
    return s;
}
inline auto operator--(Value &&ss) {
    auto s = ss;
    return --s;
}
inline auto operator--(Value &s, int) { // s--
    auto v = s.get();
    s.set(v - 1);
    return v;
}
inline auto operator--(Value &&ss, int) {
    auto s = ss;
    return s--;
}

} // namespace WebCFace
