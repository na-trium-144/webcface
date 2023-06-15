#pragma once
#include <drogon/WebSocketController.h>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include "../message/message.h"

namespace WebCFace::Server {
class Client {
  public:
    using wsConnPtr = drogon::WebSocketConnectionPtr;

  private:
    const wsConnPtr con;

    std::unordered_map<std::string, std::vector<double>> value_history;
    std::set<std::pair<std::string, std::string>> value_subsc;

  public:
    Client() = delete;
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    explicit Client(const wsConnPtr &con) : con(con) {}

    void onRecv(const std::string &msg);
    std::string name;
};
} // namespace WebCFace::Server
