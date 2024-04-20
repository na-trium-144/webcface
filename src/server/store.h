#pragma once
#include "s_client_data.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <spdlog/common.h>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Server {
using ClientDataPtr = std::shared_ptr<ClientData>;
inline struct Store {

    /*!
     * \brief 現在接続されているクライアントの一覧
     *
     */
    std::unordered_map<ClientData::wsConnPtr, ClientDataPtr> clients;
    /*!
     * \brief すべてのクライアントのデータ
     * 切断されてもデータは残り、valueやlogなどあとで参照できる
     * * 名前があるクライアントは同じ名前で再接続されたときに上書きする。
     * * (ver1.2.2から) 名前がないクライアントは切断時に削除
     *
     */
    std::unordered_map<unsigned int, ClientDataPtr> clients_by_id;

    int keep_log = 1000; // server_mainで上書きされる

    Store() : clients(), clients_by_id() {}
    ~Store() { clear(); }

    //! テスト用
    void clear();

    void newClient(const ClientData::wsConnPtr &con,
                   const std::string &remote_addr, const spdlog::sink_ptr &sink,
                   spdlog::level::level_enum level);
    void removeClient(const ClientData::wsConnPtr &con);
    ClientDataPtr getClient(const ClientData::wsConnPtr &con);

    void clientSendAll();

    /*!
     * \brief 指定したnameのclientがあればfuncを、そうでなければfunc_elseを実行
     *
     */
    void findAndDo(const std::u8string &name,
                   const std::function<void(ClientDataPtr)> &func,
                   const std::function<void()> &func_else = nullptr);
    /*!
     * \brief 指定したidのclientがあればfuncを、そうでなければfunc_elseを実行
     *
     */
    void findAndDo(unsigned int id,
                   const std::function<void(ClientDataPtr)> &func,
                   const std::function<void()> &func_else = nullptr);
    /*!
     * \brief
     * 指定したidのclientが切断前であればfuncを、そうでなければfunc_elseを実行
     *
     */
    void findConnectedAndDo(unsigned int id,
                            const std::function<void(ClientDataPtr)> &func,
                            const std::function<void()> &func_else = nullptr);
    /*!
     * \brief 各ClientDataに対して処理をする
     *
     * 切断後も含む
     *
     */
    void forEach(const std::function<void(ClientDataPtr)> &func);
    /*!
     * \brief 名前のある各ClientDataに対して処理をする
     *
     * 切断後も含む
     *
     */
    void forEachWithName(const std::function<void(ClientDataPtr)> &func);
} store;

} // namespace Server
WEBCFACE_NS_END
