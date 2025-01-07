#pragma once
#include <mutex>
#include <memory>
#include <optional>
#include "webcface/field.h"
#include "webcface/common/val_adaptor.h"

WEBCFACE_NS_BEGIN
namespace internal {

struct ResendAlways {
    template <typename T>
    static bool shouldResend(const T &, const T &) {
        return true;
    }
};
struct ResendNever {
    template <typename T>
    static bool shouldResend(const T &, const T &) {
        return false;
    }
};
struct ResendOnChange {
    template <typename T>
    static bool shouldResend(const T &prev, const T &current) {
        return *prev != *current;
    }
};

/*!
 * \brief 送受信するデータを保持するクラス
 *
 * * memberごとにフィールドを持つデータに使う。
 * member, field の2次元mapとなる
 * * T=FuncInfoの時、entryとreqは使用しない(常にすべての関数の情報が送られてくる)
 * * ImageはReqに追加情報を持つ(ReqT=ImageReq)、それ以外では使用しない(int)
 * * (ver2.6〜)
 * ValueはEntryに追加情報を持つ(EntryT=`optional<vector<size_t>>`)、
 * それ以外では使用しない(int)
 * * (ver2.6〜) ResendCondition で setSend()
 * が実際にデータを上書きするかどうかの判断基準を指定。
 *   * (ver2.5までは data_store2.cc 内の shouldSend() 関数内に記述していた)
 * * (ver2.6〜) データは std::shared_ptr<T> で保持される
 *   * DataStore2の data_send, data_send_prev, data_recv および
 *   ClientDataのsyncバッファーで同じデータが共有される。
 *   * Tがconstの場合、一度setSend(),setRecv()したデータには変更を加えられない。
 *   (新しいshared_ptrを作って再度セットする)
 *     *
 * なのでgetRecv()から取得したshared_ptrをmutexをロックしない状態で保持しても問題ない。
 * 
 * \todo
 * * Valueでoperator[]を使って値を変更した場合毎回vectorのコピーが発生していて非効率
 * * ImageFrameのshared_ptrが有効活用されていない
 * 
 */
template <typename T, typename ResendCondition = ResendAlways,
          typename ReqT = int, typename EntryT = int>
class SyncDataStore2 {
    /*!
     * \brief 次のsend時に送信するデータ。
     *
     */
    StrMap1<std::shared_ptr<T>> data_send;
    StrMap1<std::shared_ptr<T>> data_send_prev;
    /*!
     * \brief 送信済みデータ&受信済みデータ
     *
     * data_recv[member名][データ名] = 値
     *
     * ver1.11〜 setSend時には上書きされず、transferSendで上書きされる
     * それまでの間はgetRecvはdata_recvではなくdata_sendを優先的に読むようにする
     *
     */
    StrMap2<std::shared_ptr<T>> data_recv;
    /*!
     * \brief 受信済みのentry
     *
     * entry[member名] = {データ名のリスト}
     *
     */
    StrMap2<EntryT> entry;
    /*!
     * \brief データ受信リクエスト
     *
     * req[member名][データ名] が0以上ならばリクエスト済み
     * 0または未定義ならリクエストしてない
     *
     */
    StrMap2<unsigned int> req;
    /*!
     * \brief リクエストに必要なデータ
     *
     */
    StrMap2<ReqT> req_info;

    SharedString self_member_name;

  public:
    explicit SyncDataStore2(const SharedString &name);

    std::recursive_mutex mtx;

    bool isSelf(const SharedString &member) const;

    using SharedData = std::shared_ptr<T>;
    using Map1 = StrMap1<SharedData>;

    /*!
     * \brief リクエストを追加
     *
     * memberがselfの場合無効
     *
     * \return 追加した場合req_idを返し、すでにリクエストされていた場合 or
     * selfの場合 0を返す
     *
     */
    unsigned int addReq(const SharedString &member, const SharedString &field);
    /*!
     * \brief リクエストを追加
     *
     * req_infoが前回と異なっていればすでにリクエストされていても上書きする
     *
     * memberがselfの場合無効
     *
     * \return 追加した場合req_idを返し、すでにリクエストされていた場合 or
     * selfの場合 0を返す
     *
     */
    unsigned int addReq(const SharedString &member, const SharedString &field,
                        const ReqT &req_info);

    /*!
     * \brief 送信するデータをセット
     *
     * データをdata_sendとdata_recv[self_member_name]にセットし、
     * has_sendをtrueにする
     *
     */
    void setSend(const SharedString &name, const std::shared_ptr<T> &data);
    void setSend(const FieldBase &base, const std::shared_ptr<T> &data);

    /*!
     * \brief 受信したデータをdata_recvにセット
     *
     */
    void setRecv(const SharedString &from, const SharedString &name,
                 const std::shared_ptr<T> &data);
    void setRecv(const FieldBase &base, const std::shared_ptr<T> &data);
    /*!
     * \brief 受信したデータを削除
     *
     */
    void clearRecv(const SharedString &from, const SharedString &name);
    void clearRecv(const FieldBase &base);

    /*!
     * \brief data_recvからデータを返す
     *
     */
    std::shared_ptr<T> getRecv(const SharedString &from,
                               const SharedString &name);
    std::shared_ptr<T> getRecv(const FieldBase &base);
    /*!
     * \brief data_recvからデータを削除, reqを消す
     *
     * \return reqを削除したらtrue, reqがすでに削除されてればfalse
     *
     */
    bool unsetRecv(const SharedString &from, const SharedString &name);
    bool unsetRecv(const FieldBase &base);

    /*!
     * \brief memberのentryとデータをクリア
     *
     * ambiguousなので引数にFieldBaseは使わない (そもそも必要ない)
     */
    void initMember(const SharedString &from);
    /*!
     * \brief 受信したentryを追加
     *
     */
    void setEntry(const SharedString &from, const SharedString &e,
                  EntryT e_data = EntryT());
    void setEntry(const FieldBase &base,
                  EntryT e_data = EntryT());

    /*!
     * \brief entryを取得
     * \todo entryデータまで全部コピーしていて効率が悪い
     * そもそも外からmutexかけて参照取るとか?
     */
    StrMap1<EntryT> getEntry(const SharedString &from);
    StrMap1<EntryT> getEntry(const FieldBase &base);

    /*!
     * \brief req_idに対応するmember名とフィールド名を返す
     *
     */
    std::pair<SharedString, SharedString> getReq(unsigned int req_id,
                                                 const SharedString &sub_field);
    /*!
     * \brief member名とフィールド名に対応するreq_infoを返す
     *
     */
    const ReqT &getReqInfo(const SharedString &member,
                           const SharedString &field);

    /*!
     * \brief data_sendを返し、data_sendをクリア
     *
     */
    StrMap1<std::shared_ptr<T>> transferSend(bool is_first);
    StrMap1<std::shared_ptr<T>> getSendPrev(bool is_first);

    /*!
     * \brief req_sendを返し、req_sendをクリア
     *
     */
    StrMap2<unsigned int> transferReq();
};

struct FuncInfo;
struct RobotLinkData;
struct LogHistory;
} // namespace internal
class ImageFrame;
namespace message {
struct ViewData;
struct Canvas2DData;
struct Canvas3DData;
struct ImageReq;
struct ValueShape;
} // namespace message
namespace internal {
using TestStringStore =
    SyncDataStore2<const std::string, ResendOnChange>; // test用
using ValueStore = SyncDataStore2<const std::vector<double>, ResendOnChange,
                                  int, message::ValueShape>;
using TextStore = SyncDataStore2<const ValAdaptor, ResendOnChange>;
using FuncStore = SyncDataStore2<FuncInfo, ResendNever>;
using RobotModelStore =
    SyncDataStore2<const std::vector<std::shared_ptr<internal::RobotLinkData>>>;
using ImageStore =
    SyncDataStore2<const ImageFrame, ResendAlways, message::ImageReq>;
using ViewStore = SyncDataStore2<const message::ViewData>;
using Canvas3DStore = SyncDataStore2<const message::Canvas3DData>;
using Canvas2DStore = SyncDataStore2<const message::Canvas2DData>;
using LogStore = SyncDataStore2<LogHistory>;

#if WEBCFACE_SYSTEM_DLLEXPORT
extern template class SyncDataStore2<const std::string,
                                     ResendOnChange>; // test用
extern template class SyncDataStore2<const std::vector<double>, ResendOnChange,
                                     int, message::ValueShape>;
extern template class SyncDataStore2<const ValAdaptor, ResendOnChange>;
extern template class SyncDataStore2<FuncInfo, ResendNever>;
extern template class SyncDataStore2<
    const std::vector<std::shared_ptr<internal::RobotLinkData>>>;
extern template class SyncDataStore2<const ImageFrame, ResendAlways,
                                     message::ImageReq>;
extern template class SyncDataStore2<const message::ViewData>;
extern template class SyncDataStore2<const message::Canvas3DData>;
extern template class SyncDataStore2<const message::Canvas2DData>;
extern template class SyncDataStore2<LogHistory>;
#endif

} // namespace internal
WEBCFACE_NS_END
