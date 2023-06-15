#pragma once
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>

namespace drogon {
class WebSocketClient;
}
namespace WebCFace {

template <typename T>
class SyncDataStore {
  private:
    std::mutex mtx;
    std::unordered_map<std::string, T> data_send;
    std::unordered_map<std::string, std::unordered_map<std::string, T>>
        data_recv;

  public:
    void set(const std::string &name, const T &data);
    std::optional<T> try_get(const std::string &from, const std::string &name);
    std::unordered_map<std::string, T> transfer_data();
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

    void subscribe(const std::string &from, const std::string &name);

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