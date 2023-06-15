#pragma once
#include <drogon/WebSocketController.h>
#include <string>
#include "../message/message.h"

namespace WebCFace::Server {
class Client {
  public:
    using wsConnPtr = drogon::WebSocketConnectionPtr;

  private:
    const wsConnPtr con;

  public:
    Client() = delete;
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    explicit Client(const wsConnPtr &con) : con(con) {}

    void onRecv(const std::string &msg);
    std::string name;
};
} // namespace WebCFace::Server
