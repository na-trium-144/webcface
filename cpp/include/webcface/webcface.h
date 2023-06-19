#pragma once
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <set>
#include <mutex>

namespace drogon {
class WebSocketClient;
}

namespace WebCFace {

class Client;

template <typename T>
class SyncDataStore {
  private:
    std::mutex mtx;
    std::unordered_map<std::string, T> data_send;
    std::unordered_map<std::string, std::unordered_map<std::string, T>>
        data_recv;
    std::set<std::pair<std::string, std::string>> subsc, subsc_next;

  public:
    void set_send(const std::string &name, const T &data);
    void set_recv(const std::string &from, const std::string &name,
                  const T &data);
    std::optional<T> try_get_recv(const std::string &from,
                                  const std::string &name);
    std::unordered_map<std::string, T> transfer_send();
    std::set<std::pair<std::string, std::string>> transfer_subsc();
};

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
using Text = SyncData<std::string>;


class Client {
  private:
    std::shared_ptr<drogon::WebSocketClient> ws;
    bool connected = false;

    std::shared_ptr<SyncDataStore<Value::DataType>> value_store;
    std::shared_ptr<SyncDataStore<Text::DataType>> text_store;

    void onRecv(const std::string &message);

  public:
    Client() = delete;
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 80);

    void send();

    Value value(const std::string &name) { return value("", name); }
    const Value value(const std::string &from, const std::string &name) {
        return Value{value_store, from, name};
    }
    Text text(const std::string &name) { return text("", name); }
    const Text text(const std::string &from, const std::string &name) {
        return Text{text_store, from, name};
    }
};

} // namespace WebCFace
