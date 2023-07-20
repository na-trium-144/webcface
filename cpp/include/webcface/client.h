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

class Client;
struct SubjectClient {
  private:
    Client *cli;
    std::string subject;
    std::shared_ptr<SyncDataStore<Value::DataType>> value_store;
    std::shared_ptr<SyncDataStore<Text::DataType>> text_store;
    std::shared_ptr<SyncDataStore<Func::DataType>> func_store;
    std::shared_ptr<FuncStore> func_impl_store;

  public:
    SubjectClient() = default;
    SubjectClient(Client *cli, const std::string &subject);

    std::string name() const { return subject; }

    const Value value(const std::string &name) const {
        return Value{value_store, subject, name};
    }
    const Text text(const std::string &name) const {
        return Text{text_store, subject, name};
    }
    const Func func(const std::string &name) const {
        return Func{func_store, func_impl_store, cli, subject, name};
    }
    std::vector<Value> values() const {
        auto keys = value_store->getEntry(subject);
        std::vector<Value> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = value(keys[i]);
        }
        return ret;
    }
    std::vector<Text> texts() const {
        auto keys = text_store->getEntry(subject);
        std::vector<Text> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = text(keys[i]);
        }
        return ret;
    }
    std::vector<Func> funcs() const {
        auto keys = func_store->getEntry(subject);
        std::vector<Func> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = func(keys[i]);
        }
        return ret;
    }
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
    friend SubjectClient;

    Client() : Client("") {}
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 80);
    bool connected() const;
    void close();
    ~Client();

    void send();

    // ネーミングセンスがおわっている
    std::vector<SubjectClient> subjects() {
        auto keys = value_store->getEntries();
        std::vector<SubjectClient> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = subject(keys[i]);
        }
        return ret;
    }
    SubjectClient subject(const std::string &name) {
        return SubjectClient(this, name);
    }

    Value value(const std::string &name) { return subject("").value(name); }
    Text text(const std::string &name) { return subject("").text(name); }
    Func func(const std::string &name) { return subject("").func(name); }
};

} // namespace WebCFace
