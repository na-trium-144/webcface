#pragma once
#include <drogon/WebSocketController.h>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include "../message/message.h"

namespace WebCFace::Server {
class ClientData {
  public:
    using wsConnPtr = drogon::WebSocketConnectionPtr;

  private:
    const wsConnPtr con;
    bool connected() const;

    Message::Entry entry;
    std::unordered_map<std::string, std::vector<double>> value_history;
    std::set<std::pair<std::string, std::string>> value_subsc;
    std::unordered_map<std::string, std::vector<std::string>> text_history;
    std::set<std::pair<std::string, std::string>> text_subsc;

  public:
    ClientData() = delete;
    ClientData(const ClientData &) = delete;
    ClientData &operator=(const ClientData &) = delete;
    explicit ClientData(const wsConnPtr &con) : con(con) {}

    void onRecv(const std::string &msg);
    void onClose();
    void send(const std::vector<char> &m) const;
};
} // namespace WebCFace::Server
