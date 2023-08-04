#pragma once
#include <unordered_map>
#include <set>
#include <mutex>
#include <optional>

namespace WebCFace {

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
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> req;
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
    std::optional<T> getRecv(const std::string &from, const std::string &name);
    //! data_recvからデータを削除, req,req_sendをfalseにする
    void unsetRecv(const std::string &from, const std::string &name);

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

} // namespace WebCFace