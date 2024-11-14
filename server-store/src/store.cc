#include "webcface/server/store.h"
#include "webcface/server/member_data.h"
#include "webcface/common/internal/unlock.h"
#include <algorithm>

WEBCFACE_NS_BEGIN
namespace server {
void ServerStorage::clear() {
    std::unique_lock lock(store_m);
    // clients.clear();
    // clients_by_id.clear();
    while (!clients.empty()) {
        auto c_it = clients.begin();
        removeClient(c_it, lock);
    }
    MemberData::last_member_id = 0;
}
void ServerStorage::newClient(const wsConnPtr &con,
                              const std::string &remote_addr,
                              const spdlog::sink_ptr &sink,
                              spdlog::level::level_enum level) {
    auto cli =
        std::make_shared<MemberData>(this, con, remote_addr, sink, level);
    {
        std::lock_guard lock(store_m);
        clients.emplace(con, cli);
    }
    cli->onConnect();
}
void ServerStorage::removeClient(const wsConnPtr &con) {
    std::unique_lock lock(store_m);
    auto it = clients.find(con);
    if (it != clients.end()) {
        removeClient(it, lock);
    }
}
void ServerStorage::removeClient(
    std::unordered_map<wsConnPtr, MemberDataPtr>::iterator it,
    std::unique_lock<std::mutex> &lock) {
    MemberDataPtr cd = it->second;
    clients.erase(it);
    {
        ScopedUnlock un(lock);
        cd->onClose();
        // 名前があるクライアントのデータはclients_by_idに残す
        if (cd->name.empty()) {
            std::lock_guard lock2(store_m);
            clients_by_id.erase(cd->member_id);
        }
    }
}
MemberDataPtr ServerStorage::getClient(const wsConnPtr &con) {
    std::lock_guard lock(store_m);
    auto it = clients.find(con);
    if (it != clients.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}
void ServerStorage::initClientId(unsigned int id, const wsConnPtr &con) {
    auto cd = getClient(con);
    {
        std::lock_guard lock(store_m);
        clients_by_id.erase(id);
        clients_by_id.emplace(id, cd);
    }
}

void ServerStorage::clientSendAll() {
    for (const auto &cli : clientsCopy()) {
        cli.second->send();
    }
}

void ServerStorage::findAndDo(const SharedString &name,
                              const std::function<void(MemberDataPtr)> &func,
                              const std::function<void()> &func_else) {
    std::unique_lock lock(store_m);
    auto cd =
        std::find_if(clients_by_id.begin(), clients_by_id.end(),
                     [&](const auto &cd) { return cd.second->name == name; });
    if (cd != clients_by_id.end() && cd->second->sync_init) {
        ScopedUnlock un(lock);
        func(cd->second);
    } else {
        ScopedUnlock un(lock);
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void ServerStorage::findAndDo(unsigned int id,
                              const std::function<void(MemberDataPtr)> &func,
                              const std::function<void()> &func_else) {
    std::unique_lock lock(store_m);
    auto cd = clients_by_id.find(id);
    if (cd != clients_by_id.end() && cd->second->sync_init) {
        ScopedUnlock un(lock);
        func(cd->second);
    } else {
        ScopedUnlock un(lock);
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void ServerStorage::findConnectedAndDo(
    unsigned int id, const std::function<void(MemberDataPtr)> &func,
    const std::function<void()> &func_else) {
    std::unique_lock lock(store_m);
    auto cd = clients_by_id.find(id);
    if (cd != clients_by_id.end() && cd->second->sync_init &&
        cd->second->connected()) {
        ScopedUnlock un(lock);
        func(cd->second);
    } else {
        ScopedUnlock un(lock);
        if (func_else != nullptr) {
            func_else();
        }
    }
}
void ServerStorage::forEach(const std::function<void(MemberDataPtr)> &func) {
    for (const auto &cd : clientsCopy()) {
        if (cd.second->sync_init) {
            func(cd.second);
        }
    }
}
void ServerStorage::forEachWithName(
    const std::function<void(MemberDataPtr)> &func) {
    for (const auto &cd : clientsByIdCopy()) {
        if (cd.second->sync_init && !cd.second->name.empty()) {
            func(cd.second);
        }
    }
}
} // namespace server
WEBCFACE_NS_END
