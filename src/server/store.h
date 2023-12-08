#pragma once
#include "s_client_data.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <spdlog/common.h>

namespace webcface::Server {
// serverは1スレッドなのでmutexについて考える必要はない
inline struct Store {
    std::unordered_map<ClientData::wsConnPtr, std::shared_ptr<ClientData>>
        clients;
    std::unordered_map<unsigned int, std::shared_ptr<ClientData>> clients_by_id;

    int keep_log = 1000; // server_mainで上書きされる

    Store() : clients(), clients_by_id() {}

    //! テスト用
    void clear();

    void newClient(const ClientData::wsConnPtr &con,
                   const std::string &remote_addr, const spdlog::sink_ptr &sink,
                   spdlog::level::level_enum level);
    void removeClient(const ClientData::wsConnPtr &con);
    std::shared_ptr<ClientData> getClient(const ClientData::wsConnPtr &con);

    void clientSendAll();

    // 指定したnameのclientがあればfuncを、そうでなければfunc_elseを実行
    void findAndDo(const std::string &name,
                   const std::function<void(ClientData &)> &func,
                   const std::function<void()> &func_else = nullptr);
    // 指定したidのclientがあればfuncを、そうでなければfunc_elseを実行
    void findAndDo(unsigned int id,
                   const std::function<void(ClientData &)> &func,
                   const std::function<void()> &func_else = nullptr);
    //! sync_initが完了している各ClientDataに対して処理をする
    void forEach(const std::function<void(ClientData &)> &func);
    //! sync_initが完了し名前のある各ClientDataに対して処理をする
    void forEachWithName(const std::function<void(ClientData &)> &func);
} store;

} // namespace webcface::Server