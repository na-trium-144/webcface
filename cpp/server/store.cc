#include "store.h"
#include <iostream>
#include <algorithm>

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

void Store::clientSendAll() {
    for (const auto &cli : clients) {
        cli.second->send();
    }
}

void Store::findAndDo(const std::string &name,
                      const std::function<void(ClientData &)> &func,
                      const std::function<void()> &func_else) {
    auto cd =
        std::find_if(clients_by_id.begin(), clients_by_id.end(),
                     [&](const auto &cd) { return cd.second->name == name; });
    if (cd != clients_by_id.end()) {
        func(*cd->second);
    } else {
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void Store::findAndDo(unsigned int id,
                      const std::function<void(ClientData &)> &func,
                      const std::function<void()> &func_else) {
    auto cd = clients_by_id.find(id);
    if (cd != clients_by_id.end()) {
        func(*cd->second);
    } else {
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void Store::forEach(const std::function<void(ClientData &)> &func) {
    for (const auto cd : clients) {
        if (cd.second->sync_init) {
            func(*cd.second);
        }
    }
}
void Store::forEachWithName(const std::function<void(ClientData &)> &func) {
    for (const auto cd : clients_by_id) {
        if (cd.second->sync_init) {
            func(*cd.second);
        }
    }
}
} // namespace WebCFace::Server