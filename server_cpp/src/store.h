#pragma once
#include "client.h"
#include <drogon/WebSocketController.h>
#include <unordered_map>
#include <memory>

namespace WebCFace::Server {
inline class Store {
  private:
    std::unordered_map<drogon::WebSocketConnectionPtr, std::shared_ptr<Client>>
        clients;

  public:
    Store() : clients() {}
    void newClient(drogon::WebSocketConnectionPtr con);

} store;
} // namespace WebCFace::Server