#pragma once
#include "client.h"
#include <drogon/WebSocketController.h>
#include <unordered_map>
#include <memory>

namespace WebCFace::Server {
inline struct Store {
    std::unordered_map<Client::wsConnPtr, std::shared_ptr<Client>> clients;
    std::unordered_map<std::string, std::shared_ptr<Client>> clients_by_name;
    Store() : clients() {}
    void newClient(const Client::wsConnPtr &con);
    std::shared_ptr<Client> getClient(const Client::wsConnPtr &con);
} store;
} // namespace WebCFace::Server