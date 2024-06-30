#include "webcface/server/store.h"
#include "webcface/server/member_data.h"
#include <algorithm>

WEBCFACE_NS_BEGIN
namespace Server {
void ServerStorage::clear() {
    clients.clear();
    clients_by_id.clear();
    MemberData::last_member_id = 0;
}
void ServerStorage::newClient(const wsConnPtr &con,
                              const std::string &remote_addr,
                              const spdlog::sink_ptr &sink,
                              spdlog::level::level_enum level) {
    auto cli =
        std::make_shared<MemberData>(this, con, remote_addr, sink, level);
    clients.emplace(con, cli);
    cli->onConnect();
}
void ServerStorage::removeClient(const wsConnPtr &con) {
    auto it = clients.find(con);
    if (it != clients.end()) {
        it->second->onClose();
        // 名前があるクライアントのデータはclients_by_idに残す
        if (it->second->name.empty()) {
            clients_by_id.erase(it->second->member_id);
        }
        clients.erase(con);
    }
}
MemberDataPtr ServerStorage::getClient(const wsConnPtr &con) {
    auto it = clients.find(con);
    if (it != clients.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void ServerStorage::clientSendAll() {
    for (const auto &cli : clients) {
        cli.second->send();
    }
}

void ServerStorage::findAndDo(const SharedString &name,
                              const std::function<void(MemberDataPtr)> &func,
                              const std::function<void()> &func_else) {
    auto cd =
        std::find_if(clients_by_id.begin(), clients_by_id.end(),
                     [&](const auto &cd) { return cd.second->name == name; });
    if (cd != clients_by_id.end() && cd->second->sync_init) {
        func(cd->second);
    } else {
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void ServerStorage::findAndDo(unsigned int id,
                              const std::function<void(MemberDataPtr)> &func,
                              const std::function<void()> &func_else) {
    auto cd = clients_by_id.find(id);
    if (cd != clients_by_id.end() && cd->second->sync_init) {
        func(cd->second);
    } else {
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void ServerStorage::findConnectedAndDo(
    unsigned int id, const std::function<void(MemberDataPtr)> &func,
    const std::function<void()> &func_else) {
    auto cd = clients_by_id.find(id);
    if (cd != clients_by_id.end() && cd->second->sync_init &&
        cd->second->connected()) {
        func(cd->second);
    } else {
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void ServerStorage::forEach(const std::function<void(MemberDataPtr)> &func) {
    for (const auto &cd : clients) {
        if (cd.second->sync_init) {
            func(cd.second);
        }
    }
}
void ServerStorage::forEachWithName(
    const std::function<void(MemberDataPtr)> &func) {
    for (const auto &cd : clients_by_id) {
        if (cd.second->sync_init) {
            func(cd.second);
        }
    }
}
} // namespace Server
WEBCFACE_NS_END
