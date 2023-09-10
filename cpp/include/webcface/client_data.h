#pragma once
#include <vector>
#include <unordered_map>
#include <set>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <optional>
#include <string>
#include <eventpp/eventdispatcher.h>
#include <spdlog/logger.h>
#include "func_result.h"
#include "common/func.h"
#include "common/queue.h"
#include "common/view.h"
#include "common/dict.h"
#include "common/vector.h"
#include "field.h"
#include "logger.h"

namespace WebCFace {

class Value;
class Text;
class View;

struct ClientData {

    //! 送受信するデータを保持するクラス
    /*! memberごとにフィールドを持つデータに使う。
     * member, fieldの2次元mapとなる
     * T=FuncInfoの時、entryとreqは使用しない(常にすべての関数の情報が送られてくる)
     */
    template <typename T>
    class SyncDataStore2 {
        std::mutex mtx;
        //! 次のsend時に送信するデータ。
        std::unordered_map<std::string, T> data_send;
        std::unordered_map<std::string, T> data_send_prev;
        //! trueのデータは送信しない
        std::unordered_map<std::string, bool> data_send_hidden;

        //! 送信済みデータ&受信済みデータ
        /*! data_recv[member名][データ名] = 値
         */
        std::unordered_map<std::string, std::unordered_map<std::string, T>>
            data_recv;
        //! 受信済みのentry
        /*! entry[member名] = {データ名のリスト}
         */
        std::unordered_map<std::string, std::vector<std::string>> entry;
        //! データ受信リクエスト
        /*! req[member名][データ名] が0以上ならばリクエスト済み
         * 0または未定義ならリクエストしてない
         */
        std::unordered_map<std::string,
                           std::unordered_map<std::string, unsigned int>>
            req;
        //! 次のsend時に送信するデータ受信リクエスト
        /*! req[member名][データ名]
         * が1以上ならばそれをreq_idとしてリクエストをする
         * 0ならリクエストを解除する(未実装)
         */
        std::unordered_map<std::string,
                           std::unordered_map<std::string, unsigned int>>
            req_send;

        std::string self_member_name;

        void addReq(const std::string &member, const std::string &field);

      public:
        explicit SyncDataStore2(const std::string &name)
            : self_member_name(name) {}

        bool isSelf(const std::string &member) const {
            return member == self_member_name;
        }

        //! 送信するデータをdata_sendとdata_recv[self_member_name]にセット
        void setSend(const std::string &name, const T &data);
        void setSend(const FieldBase &base, const T &data) {
            setSend(base.field_, data);
        }
        void setHidden(const std::string &name, bool is_hidden);
        void setHidden(const FieldBase &base, bool is_hidden) {
            setHidden(base.field_, is_hidden);
        }
        bool isHidden(const std::string &name);
        bool isHidden(const FieldBase &base) { return isHidden(base.field_); }

        //! 受信したデータをdata_recvにセット
        void setRecv(const std::string &from, const std::string &name,
                     const T &data);
        void setRecv(const FieldBase &base, const T &data) {
            setRecv(base.member_, base.field_, data);
        }
        //! data_recvからデータを返す & req,req_sendをtrueにセット
        std::optional<T> getRecv(const std::string &from,
                                 const std::string &name);
        std::optional<T> getRecv(const FieldBase &base) {
            return getRecv(base.member_, base.field_);
        }
        //! data_recvから指定したfield以下のデータを返す
        //! 指定したfieldのreq,req_sendをtrueにセット
        //! さらに、指定したフィールド以下にデータが存在すれば
        //! そのフィールド(sub_field)も同様にreqをセット
        std::optional<Dict<T>> getRecvRecurse(const std::string &member,
                                              const std::string &field);
        std::optional<Dict<T>> getRecvRecurse(const FieldBase &base) {
            return getRecvRecurse(base.member_, base.field_);
        }
        //! data_recvからデータを削除, req,req_sendをfalseにする
        void unsetRecv(const std::string &from, const std::string &name);
        void unsetRecv(const FieldBase &base) {
            unsetRecv(base.member_, base.field_);
        }

        //! entryにmember名のみ追加
        //! ambiguousなので引数にFieldBaseは使わない (そもそも必要ない)
        void setEntry(const std::string &from);
        //! 受信したentryを追加
        void setEntry(const std::string &from, const std::string &e);

        //! entryを取得
        std::vector<std::string> getEntry(const std::string &from);
        std::vector<std::string> getEntry(const FieldBase &base) {
            return getEntry(base.member_);
        }
        //! member名のりすとを取得(entryから)
        std::vector<std::string> getMembers();

        // req_idに対応するmember名とフィールド名を返す
        std::pair<std::string, std::string>
        getReq(unsigned int req_id, const std::string &sub_field);

        //! data_sendを返し、data_sendをクリア
        std::unordered_map<std::string, T> transferSend(bool is_first);
        std::unordered_map<std::string, T> getSendPrev(bool is_first);
        //! req_sendを返し、req_sendをクリア
        std::unordered_map<std::string,
                           std::unordered_map<std::string, unsigned int>>
        transferReq(bool is_first);
    };

    template <typename T>
    class SyncDataStore1 {
        std::mutex mtx;
        std::unordered_map<std::string, T> data_recv;
        std::unordered_map<std::string, bool> req;
        std::unordered_map<std::string, bool> req_send;
        std::string self_member_name;

      public:
        explicit SyncDataStore1(const std::string &name)
            : self_member_name(name) {}

        bool isSelf(const std::string &member) const {
            return member == self_member_name;
        }

        void setRecv(const std::string &member, const T &data);

        std::optional<T> getRecv(const std::string &member);
        //! req_sendを返し、req_sendをクリア
        std::unordered_map<std::string, bool> transferReq(bool is_first);
    };

    //! AsyncFuncResultのリストを保持する。
    /*! 関数の実行結果が返ってきた時参照する
     * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
     */
    class FuncResultStore {
        std::mutex mtx;
        std::vector<AsyncFuncResult> results;

      public:
        //! 新しいcaller_idを振って新しいAsyncFuncResultを生成しそれを返す
        AsyncFuncResult &addResult(const std::string &caller,
                                   const Field &base);
        //! caller_idに対応するresultを返す
        AsyncFuncResult &getResult(int caller_id);
    };

    //! clientがsync()されたタイミングで実行中の関数を起こす
    //! さらにその関数が完了するまで待機する
    class FuncOnSync {
        std::mutex call_mtx, return_mtx;
        std::condition_variable call_cond, return_cond;

      public:
        //! sync()側が関数を起こし完了まで待機
        void sync() {
            std::unique_lock return_lock(return_mtx);
            call_cond.notify_all();
            return_cond.wait(return_lock);
        }
        //! 関数側がsync()まで待機
        void wait() {
            std::unique_lock call_lock(call_mtx);
            call_cond.wait(call_lock);
        }
        //! 関数側が完了を通知
        void done() { return_cond.notify_all(); }
    };

    explicit ClientData(const std::string &name)
        : self_member_name(name), value_store(name), text_store(name),
          func_store(name), view_store(name), log_store(name),
          sync_time_store(name), logger_sink(std::make_shared<LoggerSink>()) {
        std::vector<spdlog::sink_ptr> sinks = {logger_sink, stderr_sink};
        logger =
            std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
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
    SyncDataStore2<std::shared_ptr<std::vector<ViewComponentBase>>> view_store;
    SyncDataStore1<std::shared_ptr<std::vector<LogLine>>> log_store;
    SyncDataStore1<std::chrono::system_clock::time_point> sync_time_store;
    FuncResultStore func_result_store;

    std::unordered_map<std::string, unsigned int> member_ids;
    std::string getMemberNameFromId(unsigned int id) const;
    unsigned int getMemberIdFromName(const std::string &name) const;

    // 値を引数に持つイベント
    eventpp::EventDispatcher<FieldBaseComparable, void(Field)>
        value_change_event, text_change_event, view_change_event;
    eventpp::EventDispatcher<std::string, void(Field)> log_append_event;
    // 値は要らないイベント
    eventpp::EventDispatcher<int, void(Field)> member_entry_event;
    eventpp::EventDispatcher<std::string, void(Field)> sync_event,
        value_entry_event, text_entry_event, func_entry_event, view_entry_event;

    //! sync()を待たずに即時送って欲しいメッセージを入れるキュー
    Queue<std::string> message_queue;
    //! sync()のタイミングで実行を同期する関数のcondition_variable
    Queue<std::shared_ptr<FuncOnSync>> func_sync_queue;

    FuncWrapperType default_func_wrapper;

    //! logのキュー
    std::shared_ptr<LoggerSink> logger_sink;

    std::shared_ptr<spdlog::logger> logger, logger_internal;
};
} // namespace WebCFace