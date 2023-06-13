#include "store.h"

namespace WebCFace::Server {
void Store::newClient(const Client::wsConnPtr& con) {
    clients.emplace(con, std::make_shared<Client>(con));
}

} // namespace WebCFace::Server