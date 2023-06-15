#pragma once
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <set>

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
};

using Value = SyncData<double>;

class Client {
  private:
    std::shared_ptr<drogon::WebSocketClient> ws;
    bool connected = false;

    std::shared_ptr<SyncDataStore<Value::DataType>> value_store;

    void onRecv(const std::string &message);

  public:
    Client() = delete;
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 80);

    void send();

    Value value(const std::string &name) { return value("", name); }
    Value value(const std::string &from, const std::string &name) {
        return Value{value_store, from, name};
    }
};

} // namespace WebCFace