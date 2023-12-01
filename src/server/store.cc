#include "store.h"
#include "s_client_data.h"
#include <algorithm>

namespace webcface::Server {
void Store::clear() {
    clients.clear();
    clients_by_id.clear();
    ClientData::last_member_id = 0;
}
void Store::newClient(const ClientData::wsConnPtr &con,
                      const std::string &remote_addr,
                      const spdlog::sink_ptr &sink,
                      spdlog::level::level_enum level) {
    auto cli = std::make_shared<ClientData>(con, remote_addr, sink, level);
    clients.emplace(con, cli);
    cli->onConnect();
}
void Store::removeClient(const ClientData::wsConnPtr &con) {
    auto it = clients.find(con);
    if (it != clients.end()) {
        it->second->onClose();
        clients.erase(con);
        // clients_by_idは残す
    }
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
} // namespace webcface::Server