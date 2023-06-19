#pragma once
#include "s_client_data.h"
#include <drogon/WebSocketController.h>
#include <unordered_map>
#include <memory>

namespace WebCFace::Server {
inline struct Store {
    std::unordered_map<ClientData::wsConnPtr, std::shared_ptr<ClientData>> clients;
    std::unordered_map<std::string, std::shared_ptr<ClientData>> clients_by_name;
    Store() : clients() {}
    void newClient(const ClientData::wsConnPtr &con);
    std::shared_ptr<ClientData> getClient(const ClientData::wsConnPtr &con);
} store;
} // namespace WebCFace::Server