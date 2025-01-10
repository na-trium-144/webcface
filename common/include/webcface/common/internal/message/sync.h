#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <unordered_map>
#include <chrono>

WEBCFACE_NS_BEGIN
namespace message {

/*!
 * \brief client初期化(client->server->client)
 *
 * clientは接続後最初に1回、
 * member_name,lib_name,lib_verを送る
 *
 * member_nameが空文字列でない場合、同時に接続している他のクライアントと被ってはいけない
 * 過去に同名で接続したクライアントがある場合同じmember_idが振られる
 *
 * member_nameが空文字列の場合、他のクライアントとの被りは問題ないが、
 * 他のクライアントにはこのクライアントの存在が通知されず、
 * valueなどのデータを送ることはできない
 *
 * serverはmember_idを振り、
 * member_nameが空でなかった場合は他の全クライアントにmember_idとaddrを載せて通知する
 */
struct SyncInit : public MessageBase<MessageKind::sync_init> {
    /*!
     * \brief member名
     *
     */
    SharedString member_name;
    /*!
     * \brief member id (1以上)
     *
     */
    unsigned int member_id;
    /*!
     * \brief clientライブラリの名前(id) このライブラリでは"cpp"
     *
     * 新しくライブラリ作ることがあったら変えて識別できるようにすると良いかも
     *
     */
    std::string lib_name;
    std::string lib_ver;
    std::string addr;

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("M", member_name),
                       MSGPACK_NVP("m", member_id), MSGPACK_NVP("l", lib_name),
                       MSGPACK_NVP("v", lib_ver), MSGPACK_NVP("a", addr))
};

/*!
 * \brief serverのバージョン情報(server->client)
 *
 * (ver1.11まで: SvrVersion)
 *
 * serverはSyncInitを受信してEntryをすべて送信し終わった後にこれを返す
 *
 */
struct SyncInitEnd : public MessageBase<MessageKind::sync_init_end> {
    /*!
     * \brief serverの名前
     *
     * このライブラリでは "webcface"
     *
     */
    std::string svr_name;
    /*!
     * \brief serverのバージョン
     *
     */
    std::string ver;
    /*!
     * \brief クライアントのmember id
     * \since ver2.0
     */
    unsigned int member_id;
    /*!
     * \brief サーバーのホスト名
     * \since ver2.0
     */
    std::string hostname;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", svr_name), MSGPACK_NVP("v", ver),
                       MSGPACK_NVP("m", member_id), MSGPACK_NVP("h", hostname))
};
/*!
 * \brief ping(server->client->server)
 *
 * serverが一定間隔でこれをclientに送る
 *
 * 内容は空のmap
 *
 * clientは即座に送り返さなければならない
 * (送り返さなくても何も起きないが)
 *
 */
struct Ping : public MessageBase<MessageKind::ping> {
    Ping() = default;
};
/*!
 * \brief 各クライアントのping状況 (server->client)
 *
 */
struct PingStatus : public MessageBase<MessageKind::ping_status> {
    /*!
     * \brief member_id: ping応答時間(ms) のmap
     *
     */
    std::shared_ptr<std::unordered_map<unsigned int, int>> status;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("s", status))
};
/*!
 * \brief ping状況のリクエスト (client->server)
 *
 * これを送ると以降serverが一定間隔でPingStatusを送り返す
 *
 */
struct PingStatusReq : public MessageBase<MessageKind::ping_status_req> {
    PingStatusReq() = default;
};
/*!
 * \brief syncの時刻(client->server->client)
 *
 * clientは各sync()ごとに1回、他のメッセージより先に現在時刻を送る
 *
 * serverはそのclientのデータを1つ以上requestしているクライアントに対して
 * member_idを載せて送る
 *
 */
struct Sync : public MessageBase<MessageKind::sync> {
    unsigned int member_id; //!< member id
    /*!
     * \brief 1970/1/1 0:00(utc) からの経過ミリ秒数で表し、閏秒はカウントしない
     *
     */
    std::uint64_t time;
    Sync(unsigned int member_id,
         const std::chrono::system_clock::time_point &time)
        : member_id(member_id),
          time(std::chrono::duration_cast<std::chrono::milliseconds>(
                   time.time_since_epoch())
                   .count()) {}
    Sync() : Sync(0, std::chrono::system_clock::now()) {}
    Sync(const std::chrono::system_clock::time_point &time) : Sync(0, time) {}
    std::chrono::system_clock::time_point getTime() const {
        return std::chrono::system_clock::time_point(
            std::chrono::milliseconds(time));
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("t", time))
};

} // namespace message
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::SyncInit)
WEBCFACE_MESSAGE_FMT(webcface::message::SyncInitEnd)
WEBCFACE_MESSAGE_FMT(webcface::message::Sync)
WEBCFACE_MESSAGE_FMT(webcface::message::Ping)
WEBCFACE_MESSAGE_FMT(webcface::message::PingStatus)
WEBCFACE_MESSAGE_FMT(webcface::message::PingStatusReq)
