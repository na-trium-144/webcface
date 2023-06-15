#pragma once
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>

namespace drogon {
class WebSocketClient;
}
namespace WebCFace {

class Value;

class Client {
  private:
    std::shared_ptr<drogon::WebSocketClient> ws;
    bool connected = false;
    std::unordered_map<std::string, double> value_send;
    std::unordered_map<std::string, std::unordered_map<std::string, double>>
        value_recv;

    void subscribe(const std::string &from, const std::string &name);

  public:
    Client() = delete;
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 80);

    void send();

    Value value(const std::string &from, const std::string &name);
    Value value(const std::string &name);
    friend Value;
};

class Value {
  private:
    Client *cli;
    std::string from, name;

  public:
    Value(Client *cli, const std::string &from, const std::string &name)
        : cli(cli), from(from), name(name) {}
    Value(Client *cli, const std::string &name)
        : cli(cli), from(""), name(name) {}
    Value &set(double data);
    Value &operator=(double data) { return set(data); }
    double get() const;
    operator double() const { return get(); }
    std::optional<double> try_get() const;
};
} // namespace WebCFace