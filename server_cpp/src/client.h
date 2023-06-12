#pragma once
#include <drogon/WebSocketController.h>

namespace WebCFace::Server {
class Client {
  private:
    const drogon::WebSocketConnectionPtr con;

  public:
    Client() = delete;
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    explicit Client(drogon::WebSocketConnectionPtr con) : con(con) {}
};
} // namespace WebCFace::Server
