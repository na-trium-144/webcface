#include "store.h"
#include <iostream>

namespace WebCFace::Server {
void Store::newClient(const ClientData::wsConnPtr &con) {
    auto cli = std::make_shared<ClientData>(con);
    clients.emplace(con, cli);
    cli->onConnect();
    std::cout << "new client" << std::endl;
}
void Store::removeClient(const ClientData::wsConnPtr &con) {
    auto it = clients.find(con);
    if (it != clients.end()) {
        it->second->onClose();
        clients.erase(con);
        // clients_by_nameは残す
    }
    std::cout << "Disconnected" << std::endl;
}
std::shared_ptr<ClientData> Store::getClient(const ClientData::wsConnPtr &con) {
    auto it = clients.find(con);
    if (it != clients.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void Store::clientSendAll(){
    for(const auto &cli: clients){
        cli->second->send();
    }
}
} // namespace WebCFace::Server