#pragma once
#include <queue>
#include <vector>
#include <unordered_map>
#include <set>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <string>
#include <eventpp/eventqueue.h>
#include "func_result.h"
#include "common/func.h"
#include "event_key.h"
#include "field_base.h"

namespace WebCFace {
struct ClientData {

    //! 送受信するデータを保持するクラス
    /*! Clientが各データの種類に応じたSyncDataStoreを用意、
     * MemberとSyncDataがshared_pointerとして保持
     *
     * T=FuncInfoの時、entryとreqは使用しない(常にすべての関数の情報が送られてくる)
     */
    template <typename T>
    class SyncDataStore {
      private:
        std::mutex mtx;
        //! 次のsend時に送信するデータ。
        std::unordered_map<std::string, T> data_send;
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
        /*! req[member名][データ名] = true ならばリクエスト済み
         * falseまたは未定義ならリクエストしてない
         */
        std::unordered_map<std::string, std::unordered_map<std::string, bool>>
            req;
        //! 次のsend時に送信するデータ受信リクエスト
        /*! req[member名][データ名] = true ならばリクエストをする
         * falseならリクエストを解除する
         */
        std::unordered_map<std::string, std::unordered_map<std::string, bool>>
            req_send;

        std::string self_member_name;
        bool isSelf(const std::string &member) {
            return member == self_member_name;
        }

      public:
        SyncDataStore(const std::string &name) : self_member_name(name) {}

        //! 送信するデータをdata_sendとdata_recv[self_member_name]にセット
        void setSend(const std::string &name, const T &data);
        void setSend(const FieldBase &base, const T &data) {
            setSend(base.field_, data);
        }
        //! 受信したデータをdata_recvにセット
        void setRecv(const std::string &from, const std::string &name,
                     const T &data);
        void setRecv(const FieldBase &base, const T &data) {
            setRecv(base.member_, base.field_, data);
        }
        //! data_recvからデータを返す or なければreq,req_sendをtrueにセット
        std::optional<T> getRecv(const std::string &from,
                                 const std::string &name);
        std::optional<T> getRecv(const FieldBase &base) {
            return getRecv(base.member_, base.field_);
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

        //! data_sendを返し、data_sendをクリア
        std::unordered_map<std::string, T> transferSend();
        //! req_sendを返し、req_sendをクリア
        std::unordered_map<std::string, std::unordered_map<std::string, bool>>
        transferReq();
    };

    //! AsyncFuncResultのリストを保持する。
    /*! 関数の実行結果が返ってきた時参照する
     * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
     */
    class FuncResultStore {
      private:
        std::mutex mtx;
        std::vector<AsyncFuncResult> results;

      public:
        //! 新しいcaller_idを振って新しいAsyncFuncResultを生成しそれを返す
        AsyncFuncResult &addResult(const std::string &caller,
                                   const FieldBase &base);
        //! caller_idに対応するresultを返す
        AsyncFuncResult &getResult(int caller_id);
    };

    //! 排他制御をしたただのキュー
    template <typename T>
    class Queue {
        std::mutex mtx;
        std::condition_variable cond;
        std::queue<T> que;

      public:
        void push(const T &f) {
            {
                std::lock_guard lock(mtx);
                que.push(f);
            }
            cond.notify_one();
        }
        template <typename Dur = std::chrono::milliseconds>
        std::optional<T> pop(const Dur &d = std::chrono::milliseconds(0)) {
            std::unique_lock lock(mtx);
            if (cond.wait_for(lock, d, [this] { return !que.empty(); })) {
                auto c = que.front();
                que.pop();
                return c;
            }
            return std::nullopt;
        }
    };

    //! clientがsync()されたタイミングで実行中の関数を起こす
    //! さらにその関数が完了するまで待機する
    struct FuncOnSync {
        std::mutex call_mtx, return_mtx;
        std::condition_variable call_cond, return_cond;
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
          func_store(name) {}

    //! Client自身の名前
    std::string self_member_name;
    bool isSelf(const FieldBase &base) {
        return base.member_ == self_member_name;
    }

    SyncDataStore<double> value_store;
    SyncDataStore<std::string> text_store;
    SyncDataStore<FuncInfo> func_store;

    FuncResultStore func_result_store;

    using EventQueue = eventpp::EventQueue<EventKey, void(const EventKey &)>;
    //! 各種イベントを管理するキュー
    EventQueue event_queue;

    //! sync()を待たずに即時送って欲しいメッセージを入れるキュー
    Queue<std::vector<char>> message_queue;

    //! sync()のタイミングで実行を同期する関数のcondition_variable
    Queue<std::shared_ptr<FuncOnSync>> func_sync_queue;

    FuncWrapperType default_func_wrapper;
};
} // namespace WebCFace