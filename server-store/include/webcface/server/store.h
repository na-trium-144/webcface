#pragma once
#include <unordered_map>
#include <memory>
#include <functional>
#include <spdlog/common.h>
#include "webcface/server/server.h"
#include "webcface/common/encoding.h"

WEBCFACE_NS_BEGIN
namespace server {
struct MemberData;
using MemberDataPtr = std::shared_ptr<MemberData>;

class ServerStorage {
    std::mutex store_m;

    /*!
     * \brief 現在接続されているクライアントの一覧
     *
     */
    std::unordered_map<wsConnPtr, MemberDataPtr> clients;
    /*!
     * \brief すべてのクライアントのデータ
     * 切断されてもデータは残り、valueやlogなどあとで参照できる
     * * 名前があるクライアントは同じ名前で再接続されたときに上書きする。
     * * (ver1.2.2から) 名前がないクライアントは切断時に削除
     *
     */
    std::unordered_map<unsigned int, MemberDataPtr> clients_by_id;


    /*!
     * * onCloseを呼んでclientsから削除
     * * 名前がないものはclients_by_idからも削除
     */
    void removeClient(std::unordered_map<wsConnPtr, MemberDataPtr>::iterator it,
                      std::unique_lock<std::mutex> &lock);

  public:
    auto clientsCopy() {
        std::lock_guard lock(store_m);
        return clients;
    }
    auto clientsByIdCopy() {
        std::lock_guard lock(store_m);
        return clients_by_id;
    }

    std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status;
    Server *server;

    int keep_log;
    std::string hostname;

    explicit ServerStorage(Server *server, int keep_log = 1000)
        : clients(), clients_by_id(), ping_status(), server(server),
          keep_log(keep_log), hostname() {}
    ~ServerStorage() { clear(); }

    /*!
     * clientをすべてonCloseを呼んで削除
     */
    void clear();

    void newClient(const wsConnPtr &con, const std::string &remote_addr,
                   const spdlog::sink_ptr &sink,
                   spdlog::level::level_enum level);
    void initClientId(unsigned int id, const wsConnPtr &con);
    void removeClient(const wsConnPtr &con);
    MemberDataPtr getClient(const wsConnPtr &con);

    void clientSendAll();

    /*!
     * \brief 指定したnameのclientがあればfuncを、そうでなければfunc_elseを実行
     *
     */
    void findAndDo(const SharedString &name,
                   const std::function<void(MemberDataPtr)> &func,
                   const std::function<void()> &func_else = nullptr);
    /*!
     * \brief 指定したidのclientがあればfuncを、そうでなければfunc_elseを実行
     *
     */
    void findAndDo(unsigned int id,
                   const std::function<void(MemberDataPtr)> &func,
                   const std::function<void()> &func_else = nullptr);
    /*!
     * \brief
     * 指定したidのclientが切断前であればfuncを、そうでなければfunc_elseを実行
     *
     */
    void findConnectedAndDo(unsigned int id,
                            const std::function<void(MemberDataPtr)> &func,
                            const std::function<void()> &func_else = nullptr);
    /*!
     * \brief 各ClientDataに対して処理をする
     *
     * 切断後も含む
     *
     */
    void forEach(const std::function<void(MemberDataPtr)> &func);
    /*!
     * \brief 名前のある各ClientDataに対して処理をする
     *
     * 切断後も含む
     *
     */
    void forEachWithName(const std::function<void(MemberDataPtr)> &func);
};

} // namespace server
WEBCFACE_NS_END
