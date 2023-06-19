#include "store.h"

namespace WebCFace::Server {
void Store::newClient(const ClientData::wsConnPtr &con) {
    auto cli = std::make_shared<ClientData>(con);
    clients.emplace(con, cli);
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