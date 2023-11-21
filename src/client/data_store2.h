#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <webcface/field.h>
#include <webcface/common/dict.h>

namespace WebCFace::Internal {
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
    explicit SyncDataStore2(const std::string &name) : self_member_name(name) {}

    bool isSelf(const std::string &member) const {
        return member == self_member_name;
    }

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
    //! data_recvからデータを返す & req,req_sendをtrueにセット
    std::optional<T> getRecv(const std::string &from, const std::string &name);
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
    std::pair<std::string, std::string> getReq(unsigned int req_id,
                                               const std::string &sub_field);

    //! data_sendを返し、data_sendをクリア
    std::unordered_map<std::string, T> transferSend(bool is_first);
    std::unordered_map<std::string, T> getSendPrev(bool is_first);
    //! req_sendを返し、req_sendをクリア
    std::unordered_map<std::string,
                       std::unordered_map<std::string, unsigned int>>
    transferReq(bool is_first);
};

} // namespace WebCFace::Internal