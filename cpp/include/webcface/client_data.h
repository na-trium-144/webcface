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
         * 送信済みデータを member名="" として表す
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

      public:
        //! 送信するデータをdata_sendとdata_recv[""]にデータをセット
        void setSend(const std::string &name, const T &data);
        //! 受信したデータをdata_recvにセット
        void setRecv(const std::string &from, const std::string &name,
                     const T &data);
        //! data_recvからデータを返す or なければreq,req_sendをtrueにセット
        std::optional<T> getRecv(const std::string &from,
                                 const std::string &name);
        //! data_recvからデータを削除, req,req_sendをfalseにする
        void unsetRecv(const std::string &from, const std::string &name);

        //! entryにmember名のみ追加
        void setEntry(const std::string &from);
        //! 受信したentryを追加
        void setEntry(const std::string &from, const std::string &e);
        //! entryを取得
        std::vector<std::string> getEntry(const std::string &from);
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
        AsyncFuncResult &addResult(const std::weak_ptr<ClientData> &data,
                                   const std::string &caller,
                                   const std::string &member,
                                   const std::string &name);
        //! caller_idに対応するresultを返す
        AsyncFuncResult &getResult(int caller_id);
    };

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
        template <typename Dur>
        std::optional<T> pop(const Dur &d) {
            std::unique_lock lock(mtx);
            if (cond.wait_for(lock, d, [this] { return !que.empty(); })) {
                auto c = que.front();
                que.pop();
                return c;
            }
            return std::nullopt;
        }
    };

    SyncDataStore<double> value_store;
    SyncDataStore<std::string> text_store;
    SyncDataStore<FuncInfo> func_store;

    FuncResultStore func_result_store;

    //! 各種イベントを管理するキュー
    EventQueue event_queue;

    Queue<FuncCall> func_call_queue;
};
} // namespace WebCFace