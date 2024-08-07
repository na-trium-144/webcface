#pragma once
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <cstdlib>
#include <spdlog/logger.h>
#include "webcface/encoding/encoding.h"
#include "webcface/field.h"
#include "webcface/func_info.h"
#include "webcface/log.h"
#include "queue.h"
#include "webcface/image_frame.h"
#include "webcface/func_result.h"
#include "data_store1.h"
#include "data_store2.h"
#include "func_internal.h"

WEBCFACE_NS_BEGIN

class Log;

namespace internal {

void wsThreadMain(const std::shared_ptr<ClientData> &data);
void recvThreadMain(const std::shared_ptr<ClientData> &data);

struct ClientData : std::enable_shared_from_this<ClientData> {
    explicit ClientData(const SharedString &name,
                        const SharedString &host = nullptr, int port = -1);

    void close();

    /*!
     * \brief Client自身の名前
     *
     */
    SharedString self_member_name;
    std::optional<unsigned int> self_member_id;
    bool isSelf(const FieldBase &base) const {
        return base.member_ == self_member_name;
    }

    // std::mutex curl_m;
    SharedString host;
    int port;
    bool current_curl_connected = false;
    void *current_curl_handle = nullptr;
    std::string current_curl_path;
    std::string current_ws_buf = "";

    /*!
     * \brief websocket接続、通信するスレッド
     *
     * * 未接続状態で do_ws_initがtrue (= start()が呼ばれた) or
     * auto_reconnectがtrueのとき、再接続を行う
     * * 接続状態のとき、100usごとに受信したデータを recv_queue に入れ、
     * sync_queue 内のデータを送信する
     *
     */
    std::thread ws_thread;
    /*!
     * \brief recv_queueのメッセージを処理するスレッド
     *
     * auto_recv がtrueの場合のみ。
     *
     */
    std::thread recv_thread;

    /*!
     * \brief ws_thread, recv_thread, queue 間の同期
     *
     * closing, connected, do_ws_init, do_ws_recv
     * using_curl, sync_queue, recv_queue, sync_init_end が変化した時notifyする
     *
     */
    std::condition_variable ws_cond;
    std::mutex ws_m;
    /*!
     * close()が呼ばれたらtrue、すべてのスレッドは停止する
     */
    std::atomic<bool> closing = false;
    /*!
     * current_curl_connectedがWebSocket::initまたはrecv側の内部で変更されるので、
     * それを呼び出したスレッドがそれをこっちに反映させる
     */
    bool connected = false;
    /*!
     * SyncInitEndを受信したらtrue
     * 切断時にfalseにもどす
     */
    bool sync_init_end = false;
    /*!
     * Client側から関数が呼ばれたらtrue、
     * WebSocket::側のその関数が完了したらfalse
     *
     * trueになったときnotify
     */
    bool do_ws_init = false;
    /*!
     * Client側から関数が呼ばれたらtrue、
     * WebSocket::側のその関数が完了したらfalse
     *
     * trueになったときnotify
     */
    bool do_ws_recv = false;

    /*!
     * ただの設定フラグなのでmutexやcondとは無関係
     */
    std::atomic<bool> auto_reconnect = true, auto_recv = false;

    /*!
     * \brief 送信したいメッセージを入れるキュー
     *
     * 接続できていない場合送信されずキューにたまる
     *
     */
    std::queue<std::string> sync_queue, recv_queue;

    void message_push(std::string &&msg) {
        std::lock_guard lock(this->ws_m);
        this->sync_queue.push(std::move(msg));
        this->ws_cond.notify_all();
    }

    /*!
     * \brief 通信関係のスレッドを開始する
     *
     */
    void start();
    /*!
     * \brief threadを待機 (close時)
     *
     */
    void join();

    /*!
     * \brief recv_queueのメッセージを処理する
     *
     * * メッセージがなければtimeout後にreturn
     * * auto_reconnectがfalseで接続できてない場合はreturn (deadlock回避)
     * * timeoutがnulloptならclosingまで永遠にreturnしない
     *
     */
    void recvImpl(std::optional<std::chrono::microseconds> timeout);

    /*!
     * \brief 初期化時に送信するメッセージ
     *
     * 各種req と syncData(true) の全データが含まれる。
     *
     * ws接続直後に送信される
     *
     */
    std::string syncDataFirst();
    /*!
     * \brief sync() 1回分のメッセージ
     *
     * value, text, view, log, funcの送信データの前回からの差分が含まれる。
     * 各種reqはsyncとは無関係に送信される
     *
     * \param is_first trueのとき差分ではなく全データを送る
     * (syncDataFirst()内から呼ばれる)
     *
     */
    std::string syncData(bool is_first);
    std::string syncData(bool is_first, std::stringstream &buffer, int &len);
    /*!
     * \brief 受信時の処理
     *
     */
    void onRecv(const std::string &message);

    std::mutex entry_m;
    StrSet1 member_entry;
    SyncDataStore2<ValueData> value_store;
    SyncDataStore2<TextData> text_store;
    SyncDataStore2<FuncData> func_store;
    SyncDataStore2<ViewData> view_store;
    SyncDataStore2<ImageData, message::ImageReq> image_store;
    SyncDataStore2<RobotModelData> robot_model_store;
    SyncDataStore2<Canvas3DData> canvas3d_store;
    SyncDataStore2<Canvas2DData> canvas2d_store;
    SyncDataStore1<std::shared_ptr<std::vector<LogLineData>>> log_store;
    SyncDataStore1<std::chrono::system_clock::time_point> sync_time_store;
    FuncResultStore func_result_store;
    std::size_t log_sent_lines = 0;

    /*!
     * \brief listenerがfetchするの待ちの関数呼び出しをためておく
     *
     */
    StrMap1<Queue<FuncCallHandle>> func_listener_handlers;

    StrMap1<unsigned int> member_ids;
    std::unordered_map<unsigned int, std::string> member_lib_name,
        member_lib_ver, member_addr;
    const SharedString &getMemberNameFromId(unsigned int id) const {
        if (self_member_id && *self_member_id == id) {
            return self_member_name;
        }
        for (const auto &it : member_ids) {
            if (it.second == id) {
                return it.first;
            }
        }
        static SharedString empty;
        return empty;
    }
    unsigned int getMemberIdFromName(const SharedString &name) const {
        if (name == self_member_name && self_member_id) {
            return *self_member_id;
        }
        auto it = member_ids.find(name);
        if (it != member_ids.end()) {
            return it->second;
        }
        return 0;
    }

    /*!
     * \brief コールバックリスト(のmapなど)にアクセスするときにロック
     *
     */
    std::mutex event_m;
    /*!
     * 各種イベントはコールバックを登録しようとした時(EventTargetの初期化時)
     * にはじめて初期化する。
     *
     * なのでmapになっていないmember_entry_eventもnullの可能性がある
     */
    std::shared_ptr<std::function<void(Member)>> member_entry_event;

    StrMap2<std::shared_ptr<std::function<void(Value)>>> value_change_event;
    StrMap2<std::shared_ptr<std::function<void(Text)>>> text_change_event;
    StrMap2<std::shared_ptr<std::function<void(Image)>>> image_change_event;
    StrMap2<std::shared_ptr<std::function<void(RobotModel)>>>
        robot_model_change_event;
    StrMap2<std::shared_ptr<std::function<void(View)>>> view_change_event;
    StrMap2<std::shared_ptr<std::function<void(Canvas3D)>>>
        canvas3d_change_event;
    StrMap2<std::shared_ptr<std::function<void(Canvas2D)>>>
        canvas2d_change_event;
    StrMap1<std::shared_ptr<std::function<void(Log)>>> log_append_event;
    StrMap1<std::shared_ptr<std::function<void(Member)>>> sync_event,
        ping_event;
    StrMap1<std::shared_ptr<std::function<void(Value)>>> value_entry_event;
    StrMap1<std::shared_ptr<std::function<void(Text)>>> text_entry_event;
    StrMap1<std::shared_ptr<std::function<void(Func)>>> func_entry_event;
    StrMap1<std::shared_ptr<std::function<void(View)>>> view_entry_event;
    StrMap1<std::shared_ptr<std::function<void(Image)>>> image_entry_event;
    StrMap1<std::shared_ptr<std::function<void(RobotModel)>>>
        robot_model_entry_event;
    StrMap1<std::shared_ptr<std::function<void(Canvas3D)>>>
        canvas3d_entry_event;
    StrMap1<std::shared_ptr<std::function<void(Canvas2D)>>>
        canvas2d_entry_event;

    std::shared_ptr<spdlog::logger> logger_internal;
    std::unique_ptr<std::streambuf> logger_buf;
    std::unique_ptr<std::ostream> logger_os;
    std::unique_ptr<std::wstreambuf> logger_buf_w;
    std::unique_ptr<std::wostream> logger_os_w;

    std::string svr_name, svr_version, svr_hostname;

    std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status =
        nullptr;
    bool ping_status_req = false;
    /*!
     * \brief ping_status_reqをtrueにしmessage_queueに投げる
     *
     */
    void pingStatusReq();
};
} // namespace internal
WEBCFACE_NS_END
