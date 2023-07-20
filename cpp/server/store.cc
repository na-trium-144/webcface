#include "store.h"
#include "../controllers/WebCFace_Server_MainWebsock.h"

namespace WebCFace::Server {
void controllerKeeper() {
    // static libraryでControllerが消滅しないようにするため、
    // Controllerを使用するだけの関数をつくる
    MainWebsock m;
}

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

} // namespace WebCFace::Server