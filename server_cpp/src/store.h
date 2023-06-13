#pragma once
#include "client.h"
#include <drogon/WebSocketController.h>
#include <unordered_map>
#include <memory>

namespace WebCFace::Server {
inline class Store {
  private:
    std::unordered_map<Client::wsConnPtr, std::shared_ptr<Client>> clients;

  public:
    Store() : clients() {}
    void newClient(const Client::wsConnPtr &con);
    std::shared_ptr<Client> getClient(const Client::wsConnPtr &con);
} store;
} // namespace WebCFace::Server