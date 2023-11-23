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

    //! Client自身の名前
    std::string self_member_name;
    bool isSelf(const FieldBase &base) const {
        return base.member_ == self_member_name;
    }

    std::string host;
    int port;

    //! websocket通信するスレッド
    std::unique_ptr<std::thread> message_thread;
    //! recv_queueを処理するスレッド
    std::unique_ptr<std::thread> recv_thread;

    //! thisに依存するものを遅れて初期化する
    /*!
     * * 初期化されるのはthreadと、logger関係
     * * Clientのコンストラクタが生成する場合はClientのコンストラクタが呼ぶ
     * * port < 0 のときwebsocket通信はしない
     */
    WEBCFACE_DLL void start();
    //! threadを待機 (close時)
    void join() {
        message_thread->join();
        recv_thread->join();
    }

    //! 受信時の処理
    WEBCFACE_DLL void onRecv(const std::string &message);

    SyncDataStore2<std::shared_ptr<VectorOpt<double>>> value_store;
    SyncDataStore2<std::shared_ptr<std::string>> text_store;
    SyncDataStore2<std::shared_ptr<FuncInfo>> func_store;
    SyncDataStore2<std::shared_ptr<std::vector<Common::ViewComponentBase>>>
        view_store;
    SyncDataStore1<std::shared_ptr<std::vector<std::shared_ptr<LogLine>>>>
        log_store;
    SyncDataStore1<std::chrono::system_clock::time_point> sync_time_store;
    FuncResultStore func_result_store;

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

    // 値を引数に持つイベント
    eventpp::EventDispatcher<FieldBaseComparable, void(Field)>
        value_change_event, text_change_event, view_change_event;
    eventpp::EventDispatcher<std::string, void(Field)> log_append_event;
    // 値は要らないイベント
    eventpp::EventDispatcher<int, void(Field)> member_entry_event;
    eventpp::EventDispatcher<std::string, void(Field)> sync_event,
        value_entry_event, text_entry_event, func_entry_event, view_entry_event,
        ping_event;

    //! sync()を待たずに即時送って欲しいメッセージを入れるキュー
    Common::Queue<std::string> message_queue;
    //! wsが受信したメッセージを入れるキュー
    Common::Queue<std::string> recv_queue;
    //! sync()のタイミングで実行を同期する関数のcondition_variable
    Common::Queue<std::shared_ptr<FuncOnSync>> func_sync_queue;

    FuncWrapperType default_func_wrapper;

    //! logのキュー
    std::shared_ptr<LoggerSink> logger_sink;
    std::shared_ptr<spdlog::logger> logger, logger_internal;
    std::unique_ptr<LoggerBuf> logger_buf;
    std::unique_ptr<std::ostream> logger_os;

    //! close()が呼ばれたらtrue
    std::atomic<bool> closing = false;
    std::atomic<bool> connected_ = false;
    //! 初回のsync()で全データを送信するがそれが完了したかどうか
    //! 再接続したらfalseに戻す
    std::atomic<bool> sync_init = false;

    //! serverの情報
    std::string svr_name, svr_version;

    std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status =
        nullptr;
    bool ping_status_req = false;
    bool ping_status_req_send = false;
};
} // namespace webcface::Internal