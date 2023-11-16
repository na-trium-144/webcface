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

namespace WebCFace::Internal {

struct ClientData {
    explicit ClientData(const std::string &name)
        : self_member_name(name), value_store(name), text_store(name),
          func_store(name), view_store(name), log_store(name),
          sync_time_store(name), logger_sink(std::make_shared<LoggerSink>()) {
        std::vector<spdlog::sink_ptr> sinks = {logger_sink, stderr_sink};
        logger =
            std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        logger_internal = std::make_shared<spdlog::logger>(
            "webcface_internal(" + name + ")", stderr_sink);
        logger_internal->set_level(logger_internal_level);
    }

    //! Client自身の名前
    std::string self_member_name;
    bool isSelf(const FieldBase &base) const {
        return base.member_ == self_member_name;
    }

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
} // namespace WebCFace::Internal