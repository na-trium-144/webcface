#pragma once
#include <drogon/WebSocketController.h>
#include <string>
#include <unordered_map>
#include <set>
#include "../message/message.h"

namespace WebCFace::Server {
class ClientData {
  public:
    using wsConnPtr = drogon::WebSocketConnectionPtr;

  private:
    const wsConnPtr con;
    bool connected() const;

    std::string name;
    std::unordered_map<std::string, double> value;
    std::unordered_map<std::string, std::string> text;
    std::unordered_map<std::string, Message::FuncInfo> func;
    std::set<std::pair<std::string, std::string>> value_subsc;
    std::set<std::pair<std::string, std::string>> text_subsc;

  public:
    ClientData() = delete;
    ClientData(const ClientData &) = delete;
    ClientData &operator=(const ClientData &) = delete;
    explicit ClientData(const wsConnPtr &con) : con(con) {}

    void onConnect();
    void onRecv(const std::string &msg);
    void onClose();
    void send(const std::vector<char> &m) const;
};
} // namespace WebCFace::Server
