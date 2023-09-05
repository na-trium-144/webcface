#pragma once
#include "s_client_data.h"
#include <unordered_map>
#include <memory>
#include <functional>

namespace WebCFace::Server {
// serverは1スレッドなのでmutexについて考える必要はない
inline struct Store {
    std::unordered_map<ClientData::wsConnPtr, std::shared_ptr<ClientData>>
        clients;
    std::unordered_map<std::string, std::shared_ptr<ClientData>>
        clients_by_name;
    Store() : clients() {}
    void newClient(const ClientData::wsConnPtr &con);
    void removeClient(const ClientData::wsConnPtr &con);
    std::shared_ptr<ClientData> getClient(const ClientData::wsConnPtr &con);

    void clientSendAll();

    // 指定したnameのclientがあればfuncを、そうでなければfunc_elseを実行
    void
    findAndDo(const std::string &name,
              const std::function<void(ClientData &)> &func,
              const std::function<void()> &func_else = nullptr);
    //! sync_initが完了している各ClientDataに対して処理をする
    void forEach(const std::function<void(ClientData &)> &func);
    //! sync_initが完了し名前のある各ClientDataに対して処理をする
    void forEachWithName(const std::function<void(ClientData &)> &func);
} store;

} // namespace WebCFace::Server