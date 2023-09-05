#pragma once
#include "s_client_data.h"
#include <unordered_map>
#include <memory>

namespace WebCFace::Server {
// serverは1スレッドなのでmutexについて考える必要はない
inline struct Store {
    std::unordered_map<ClientData::wsConnPtr, std::shared_ptr<ClientData>>
        clients;
    std::unordered_map<std::string, std::shared_ptr<ClientData>>
        clients_by_name;
    Store() : clients() {}
    void newClient(const ClientData::wsConnPtr &con);
    void removeClient(const ClientData::wsConnPtr &con);
    std::shared_ptr<ClientData> getClient(const ClientData::wsConnPtr &con);

    void clientSendInit();
    void clientSendAll();
} store;

} // namespace WebCFace::Server