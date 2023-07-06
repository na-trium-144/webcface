#pragma once
#include <memory>
#include <string>
#include <future>
#include <vector>

namespace drogon {
class WebSocketClient;
}

namespace WebCFace {

// SyncDataStore<Entry> はnameを""にして運用
struct Entry {
    std::vector<std::string> value;
    std::vector<std::string> text;
};

class Client {
  private:
    std::shared_ptr<drogon::WebSocketClient> ws;
    bool closing = false;
    std::future<void> connection_finished;
    void reconnect();

    std::string name;
    std::string host;
    int port;

    std::shared_ptr<SyncDataStore<Entry>> entry_store =
        std::make_shared<SyncDataStore<Entry>>();
    std::shared_ptr<SyncDataStore<Value::DataType>> value_store =
        std::make_shared<SyncDataStore<Value::DataType>>();
    std::shared_ptr<SyncDataStore<Text::DataType>> text_store =
        std::make_shared<SyncDataStore<Text::DataType>>();
    std::shared_ptr<SyncDataStore<Func::DataType>> func_store =
        std::make_shared<SyncDataStore<Func::DataType>>();

    std::shared_ptr<FuncStore> func_impl_store = std::make_shared<FuncStore>();

    void onRecv(const std::string &message);
    void send(const std::vector<char> &m);

  public:
    friend Func;

    Client() : Client("") {}
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 80);
    bool connected() const;
    void close();
    ~Client();

    void send();

    std::vector<std::string> getClientList() {
        return entry_store->get_recv_key();
    }
    Entry getEntry(const std::string &from) {
        auto e = entry_store->try_get_recv(from, "");
        if (e) {
            return *e;
        } else {
            return Entry{};
        }
    }

    Value value(const std::string &name) { return value("", name); }
    const Value value(const std::string &from, const std::string &name) {
        return Value{value_store, from, name};
    }
    Text text(const std::string &name) { return text("", name); }
    const Text text(const std::string &from, const std::string &name) {
        return Text{text_store, from, name};
    }
    Func func(const std::string &name) { return func("", name); }
    const Func func(const std::string &from, const std::string &name) {
        return Func{func_store, func_impl_store, this, from, name};
    }
};

} // namespace WebCFace
