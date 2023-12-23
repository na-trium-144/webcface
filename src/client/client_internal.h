#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <cstdlib>
#include <eventpp/eventdispatcher.h>
#include <spdlog/logger.h>
#include <webcface/common/vector.h>
#include <webcface/field.h>
#include <webcface/common/func.h>
#include <webcface/common/view.h>
#include <webcface/common/log.h>
#include <webcface/common/queue.h>
#include <webcface/func_result.h>
#include <webcface/logger.h>
#include "data_store1.h"
#include "data_store2.h"
#include "func_internal.h"

namespace webcface::Internal {

WEBCFACE_DLL void messageThreadMain(std::shared_ptr<ClientData> data,
                                    std::string host, int port);

WEBCFACE_DLL void recvThreadMain(std::shared_ptr<ClientData> data);

struct ClientData : std::enable_shared_from_this<ClientData> {
    WEBCFACE_DLL explicit ClientData(const std::string &name,
                                     const std::string &host = "",
                                     int port = -1);

    /*!
     * \brief Client自身の名前
     *
     */
    std::string self_member_name;
    bool isSelf(const FieldBase &base) const {
        return base.member_ == self_member_name;
    }

    std::string host;
    int port;

    /*!
     * \brief websocket通信するスレッド
     *
     */
    std::unique_ptr<std::thread> message_thread;
    /*!
     * \brief recv_queueを処理するスレッド
     *
     */
    std::unique_ptr<std::thread> recv_thread;

    /*!
     * \brief close()が呼ばれたらtrue
     *
     */
    std::atomic<bool> closing = false;
    std::atomic<bool> connected = false;
    std::mutex connect_state_m;
    std::condition_variable connect_state_cond;

    /*!
     * \brief 通信関係のスレッドを開始する
     *
     */
    WEBCFACE_DLL void start();
    /*!
     * \brief threadを待機 (close時)
     *
     */
    WEBCFACE_DLL void join();

    /*!
     * \brief 初期化時に送信するメッセージをキューに入れる
     *
     * 各種req と syncData(true) の全データが含まれる。
     *
     * コンストラクタ直後start()前と、切断直後に生成してキューの最初に入れる
     *
     */
    WEBCFACE_DLL void syncDataFirst();
    /*!
     * \brief sync() 1回分のメッセージをキューに入れる
     *
     * value, text, view, log, funcの送信データの前回からの差分が含まれる。
     * 各種reqはsyncとは無関係に送信される
     *
     * \param is_first trueのとき差分ではなく全データを送る
     * (syncDataFirst()内から呼ばれる)
     *
     */
    WEBCFACE_DLL void syncData(bool is_first);

    /*!
     * \brief 送信したいメッセージを入れるキュー
     *
     * 接続できていない場合送信されずキューにたまる
     *
     */
    std::shared_ptr<Common::Queue<std::string>> message_queue;
    /*!
     * \brief wsが受信したメッセージを入れるキュ
     *
     */
    Common::Queue<std::string> recv_queue;

    /*!
     * \brief 受信時の処理
     *
     */
    WEBCFACE_DLL void onRecv(const std::string &message);

    SyncDataStore2<std::shared_ptr<VectorOpt<double>>> value_store;
    SyncDataStore2<std::shared_ptr<std::string>> text_store;
    SyncDataStore2<std::shared_ptr<FuncInfo>> func_store;
    SyncDataStore2<std::shared_ptr<std::vector<Common::ViewComponentBase>>>
        view_store;
    std::shared_ptr<SyncDataStore1<std::shared_ptr<std::vector<LogLine>>>>
        log_store;
    SyncDataStore1<std::chrono::system_clock::time_point> sync_time_store;
    FuncResultStore func_result_store;
    std::size_t log_sent_lines = 0;

    /*!
     * \brief listenerがfetchするの待ちの関数呼び出しをためておく
     *
     */
    std::unordered_map<std::string, Common::Queue<FuncCallHandle>>
        func_listener_handlers;

    std::unordered_map<std::string, unsigned int> member_ids;
    std::unordered_map<unsigned int, std::string> member_lib_name,
        member_lib_ver, member_addr;
    std::string getMemberNameFromId(unsigned int id) const {
        for (const auto &it : member_ids) {
            if (it.second == id) {
                return it.first;
            }
        }
        return "";
    }
    unsigned int getMemberIdFromName(const std::string &name) const {
        auto it = member_ids.find(name);
        if (it != member_ids.end()) {
            return it->second;
        }
        return 0;
    }

    eventpp::EventDispatcher<FieldBaseComparable, void(Field)>
        value_change_event, text_change_event, view_change_event;
    eventpp::EventDispatcher<std::string, void(Field)> log_append_event;
    eventpp::EventDispatcher<int, void(Field)> member_entry_event;
    eventpp::EventDispatcher<std::string, void(Field)> sync_event,
        value_entry_event, text_entry_event, func_entry_event, view_entry_event,
        ping_event;

    /*!
     * \brief sync()のタイミングで実行を同期する関数のcondition_variable
     *
     */
    Common::Queue<std::shared_ptr<FuncOnSync>> func_sync_queue;

    FuncWrapperType default_func_wrapper;

    std::shared_ptr<LoggerSink> logger_sink;
    std::shared_ptr<spdlog::logger> logger, logger_internal;
    std::unique_ptr<LoggerBuf> logger_buf;
    std::unique_ptr<std::ostream> logger_os;

    std::string svr_name, svr_version;

    std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status =
        nullptr;
    bool ping_status_req = false;
    /*!
     * \brief ping_status_reqをtrueにしmessage_queueに投げる
     *
     */
    WEBCFACE_DLL void pingStatusReq();
};
} // namespace webcface::Internal