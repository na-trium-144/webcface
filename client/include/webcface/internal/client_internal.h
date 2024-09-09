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
// void syncThreadMain(const std::shared_ptr<ClientData> &data);

extern std::atomic<int> log_keep_lines;

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
    // /*!
    //  * \brief recv_queueのメッセージを処理するスレッド
    //  *
    //  * auto_recv がtrueの場合のみ。
    //  *
    //  */
    // std::thread sync_thread;

    /*!
     * \brief ws_thread, recv_thread, queue 間の同期
     *
     * closing, connected, do_ws_init, do_ws_recv,
     * sync_queue, recv_queue, sync_init_end が変化した時notifyする
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
     * WebSocket::側のinit関数が完了したらfalse
     *
     * trueになったときnotify
     */
    bool do_ws_init = false;
    /*!
     * Client側から関数が呼ばれたらtrue、
     * WebSocket::側のrecv関数が完了したらfalse
     *
     * true,falseになったときnotify
     *
     * recv_readyの間にdo_ws_recvを立て、
     * recv()を行い、これがfalseになったことで完了したことを知る
     */
    bool do_ws_recv = false;
    /*!
     * recv待機中true
     * WebSocket::側のrecv関数が完了したらfalse
     */
    bool recv_ready = false;


    /*!
     * ただの設定フラグなのでmutexやcondとは無関係
     */
    std::atomic<bool> auto_reconnect = true;

    std::queue<std::vector<std::pair<int, std::shared_ptr<void>>>> recv_queue;

    struct SyncDataSnapshot {
        std::chrono::system_clock::time_point time;
        StrMap1<ValueData> value_data;
        StrMap1<TextData> text_data;
        StrMap1<RobotModelData> robot_model_data;
        StrMap1<ViewData> view_prev, view_data;
        StrMap1<Canvas3DData> canvas3d_prev, canvas3d_data;
        StrMap1<Canvas2DData> canvas2d_prev, canvas2d_data;
        StrMap1<ImageData> image_data;
        std::vector<LogLineData> log_data;
        StrMap1<FuncData> func_data;
    };
    /*!
     * \brief 送信したいメッセージを入れるキュー
     *
     * 接続できていない場合送信されずキューにたまる
     *
     * msgpackのシリアライズに時間がかかるので、
     * sync()時はシリアライズ後のメッセージではなく
     * 必要なデータを含んだSyncDataSnapshotをpushし(syncData()),
     * あとで別スレッドでそれをメッセージにする (packSyncData())
     *
     */
    std::queue<std::variant<std::string, SyncDataSnapshot>> sync_queue;

    struct SyncDataFirst {
        StrMap2<unsigned int> value_req, text_req, robot_model_req, view_req,
            canvas3d_req, canvas2d_req, image_req;
        StrMap2<message::ImageReq> image_req_info;
        StrMap1<bool> log_req;
        bool ping_status_req;
        SyncDataSnapshot sync_data;
    };
    /*!
     * 次回接続後一番最初に送信するメッセージ
     *
     * * syncDataFirst() の返り値であり、
     * すべてのリクエストとすべてのsyncデータ(1時刻分)が含まれる
     * * sync()時に未接続かつこれが空ならその時点のsyncDataFirstをこれにセット
     * * 接続時にこれが空でなければ、
     *   * これ + sync_queueの中身(=syncDataFirst以降のすべてのsync()データ)
     * を、
     *   * これが空ならその時点のsyncDataFirstを、
     * * 送信する
     * * 送信したら再度これを空にする
     */
    std::optional<SyncDataFirst> sync_first;

    /*!
     * sync_firstとsyncData(),syncDataFirst()呼び出しをガードするmutex
     */
    std::mutex sync_m;

    /*!
     * 接続中の場合メッセージをキューに入れtrueを返し、
     * 接続していない場合なにもせずfalseを返す
     *
     * 未接続の間送る必要のないデータに使う。
     * Req, Callなど
     */
    bool messagePushOnline(std::string &&msg) {
        std::lock_guard lock(this->ws_m);
        if (this->connected) {
            this->sync_queue.push(std::move(msg));
            this->ws_cond.notify_all();
            return true;
        } else {
            return false;
        }
    }
    /*!
     * 接続中かどうかに関係なくメッセージをキューに入れる
     *
     * syncで確実に送信するデータに使う。
     */
    void messagePushAlways(std::string &&msg) {
        std::lock_guard lock(this->ws_m);
        this->sync_queue.push(std::move(msg));
        this->ws_cond.notify_all();
    }
    void messagePushAlways(SyncDataSnapshot &&msg) {
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
    void syncImpl(bool sync, bool forever,
                  std::optional<std::chrono::microseconds> timeout);

    /*!
     * \brief 初期化時に送信するメッセージ
     *
     * 各種req と syncData(true) の全データが含まれる。
     *
     * 変数 sync_first の説明を参照
     *
     */
    SyncDataFirst syncDataFirst();
    std::string packSyncDataFirst(const SyncDataFirst &data);
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
    SyncDataSnapshot syncData(bool is_first);
    std::string packSyncData(std::stringstream &buffer, int &len,
                             const SyncDataSnapshot &data);
    /*!
     * \brief 受信時の処理
     *
     */
    void
    onRecv(const std::vector<std::pair<int, std::shared_ptr<void>>> &messages);

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
    SyncDataStore1<std::shared_ptr<std::deque<LogLineData>>> log_store;
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
    StrMap2<std::shared_ptr<std::function<void(Variant)>>> text_change_event;
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
