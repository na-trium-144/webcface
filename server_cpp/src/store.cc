#include "store.h"

namespace WebCFace::Server {
void Store::newClient(drogon::WebSocketConnectionPtr con) {
    clients.emplace(con, std::make_shared<Client>(con));
}
} // namespace WebCFace::Server