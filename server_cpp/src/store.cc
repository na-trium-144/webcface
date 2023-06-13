#include "store.h"

namespace WebCFace::Server {
void Store::newClient(const Client::wsConnPtr &con) {
    clients.emplace(con, std::make_shared<Client>(con));
}
std::shared_ptr<Client> Store::getClient(const Client::wsConnPtr &con) {
    auto it = clients.find(con);
    if (it != clients.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

} // namespace WebCFace::Server