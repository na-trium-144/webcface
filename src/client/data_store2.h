#pragma once
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <optional>
#include <functional>
#include <concepts>
#include <webcface/field.h>
#include <webcface/common/dict.h>
#include <webcface/common/def.h>
#include <webcface/common/val.h>
#include <webcface/common/func.h>
#include <webcface/common/view.h>
#include <webcface/common/image.h>
#include <webcface/common/robot_model.h>
#include <webcface/common/canvas3d.h>
#include <webcface/common/canvas2d.h>
#include "../message/message.h"

WEBCFACE_NS_BEGIN
namespace Internal {
/*!
 * \brief 送受信するデータを保持するクラス
 *
 * memberごとにフィールドを持つデータに使う。
 * member, fieldの2次元mapとなる
 *
 * T=FuncInfoの時、entryとreqは使用しない(常にすべての関数の情報が送られてくる)
 *
 */
template <typename T, typename ReqT = int>
class WEBCFACE_DLL SyncDataStore2 {
    /*!
     * \brief 次のsend時に送信するデータ。
     *
     */
    std::unordered_map<std::u8string, T> data_send;
    std::unordered_map<std::u8string, T> data_send_prev;
    /*!
     * \brief 送信済みデータ&受信済みデータ
     *
     * data_recv[member名][データ名] = 値
     *
     * ver1.11〜 setSend時には上書きされず、transferSendで上書きされる
     * それまでの間はgetRecvはdata_recvではなくdata_sendを優先的に読むようにする
     *
     */
    std::unordered_map<std::u8string, std::unordered_map<std::u8string, T>>
        data_recv;
    /*!
     * \brief 受信済みのentry
     *
     * entry[member名] = {データ名のリスト}
     *
     */
    std::unordered_map<std::u8string, std::unordered_set<std::u8string>> entry;
    /*!
     * \brief データ受信リクエスト
     *
     * req[member名][データ名] が0以上ならばリクエスト済み
     * 0または未定義ならリクエストしてない
     *
     */
    std::unordered_map<std::u8string,
                       std::unordered_map<std::u8string, unsigned int>>
        req;
    /*!
     * \brief リクエストに必要なデータ
     *
     */
    std::unordered_map<std::u8string, std::unordered_map<std::u8string, ReqT>>
        req_info;

    std::u8string self_member_name;


  public:
    explicit SyncDataStore2(const std::u8string &name)
        : self_member_name(name) {}

    std::recursive_mutex mtx;

    bool isSelf(std::u8string_view member) const {
        return member == self_member_name;
    }

    /*!
     * \brief リクエストを追加
     *
     * memberがselfの場合無効
     *
     * \return 追加した場合req_idを返し、すでにリクエストされていた場合 or
     * selfの場合 0を返す
     *
     */
    unsigned int addReq(const std::u8string &member,
                        const std::u8string &field);
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
    unsigned int addReq(const std::u8string &member, const std::u8string &field,
                        const ReqT &req_info);

    /*!
     * \brief 送信するデータをセット
     *
     * データをdata_sendとdata_recv[self_member_name]にセットし、
     * has_sendをtrueにする
     *
     */
    void setSend(const std::u8string &name, const T &data);
    void setSend(const FieldBase &base, const T &data) {
        setSend(base.field_, data);
    }

    /*!
     * \brief 受信したデータをdata_recvにセット
     *
     */
    void setRecv(const std::u8string &from, const std::u8string &name,
                 const T &data);
    void setRecv(const FieldBase &base, const T &data) {
        setRecv(base.member_, base.field_, data);
    }
    /*!
     * \brief 受信したデータを削除
     *
     */
    void clearRecv(const std::u8string &from, const std::u8string &name);
    void clearRecv(const FieldBase &base) {
        clearRecv(base.member_, base.field_);
    }

    /*!
     * \brief data_recvからデータを返す
     *
     */
    std::optional<T> getRecv(const std::u8string &from,
                             const std::u8string &name);
    std::optional<T> getRecv(const FieldBase &base) {
        return getRecv(base.member_, base.field_);
    }
    /*!
     * \brief data_recvから指定したfield以下のデータを返す
     *
     * 指定したfieldのreq,req_sendをtrueにセット
     * さらに、指定したフィールド以下にデータが存在すれば
     * そのフィールド(sub_field)も同様にreqをセット
     *
     * \param cb 再帰的に呼び出す
     *
     */
    std::optional<Dict<T>> getRecvRecurse(
        const std::u8string &member, const std::u8string &field,
        const std::function<void(const std::u8string &)> &cb = nullptr);
    std::optional<Dict<T>> getRecvRecurse(
        const FieldBase &base,
        const std::function<void(const std::u8string &)> &cb = nullptr) {
        return getRecvRecurse(base.member_, base.field_, cb);
    }
    /*!
     * \brief data_recvからデータを削除, reqを消す
     *
     * \return reqを削除したらtrue, reqがすでに削除されてればfalse
     *
     */
    bool unsetRecv(const std::u8string &from, const std::u8string &name);
    bool unsetRecv(const FieldBase &base) {
        return unsetRecv(base.member_, base.field_);
    }

    /*!
     * \brief memberのentryをクリア
     *
     * ambiguousなので引数にFieldBaseは使わない (そもそも必要ない)
     */
    void clearEntry(const std::u8string &from);
    /*!
     * \brief 受信したentryを追加
     *
     */
    void setEntry(const std::u8string &from, const std::u8string &e);

    /*!
     * \brief entryを取得
     *
     */
    std::unordered_set<std::u8string> getEntry(const std::u8string &from);
    std::unordered_set<std::u8string> getEntry(const FieldBase &base) {
        return getEntry(base.member_);
    }

    /*!
     * \brief req_idに対応するmember名とフィールド名を返す
     *
     */
    std::pair<std::u8string, std::u8string>
    getReq(unsigned int req_id, const std::u8string &sub_field);
    /*!
     * \brief member名とフィールド名に対応するreq_infoを返す
     *
     */
    const ReqT &getReqInfo(const std::u8string &member,
                           const std::u8string &field);

    /*!
     * \brief data_sendを返し、data_sendをクリア
     *
     */
    std::unordered_map<std::u8string, T> transferSend(bool is_first);
    std::unordered_map<std::u8string, T> getSendPrev(bool is_first);

    /*!
     * \brief req_sendを返し、req_sendをクリア
     *
     */
    std::unordered_map<std::u8string,
                       std::unordered_map<std::u8string, unsigned int>>
    transferReq();

    template <typename ElemT>
    std::shared_ptr<std::unordered_map<int, ElemT>>
    getDiff(std::vector<ElemT> *current, std::vector<ElemT> *prev = nullptr) {
        auto v_diff = std::make_shared<std::unordered_map<int, ElemT>>();
        for (std::size_t i = 0; i < current->size(); i++) {
            if (!prev || prev->size() <= i || (*prev)[i] != (*current)[i]) {
                v_diff->emplace(static_cast<int>(i), (*current)[i]);
            }
        }
        return v_diff;
    }
};

using ValueData = std::shared_ptr<VectorOpt<double>>;
using TextData = std::shared_ptr<Common::ValAdaptor>;
using FuncData = std::shared_ptr<FuncInfo>;
using ViewData = std::shared_ptr<std::vector<Common::ViewComponentBase>>;
using RobotModelData = std::shared_ptr<std::vector<Common::RobotLink>>;
using Canvas3DData =
    std::shared_ptr<std::vector<Common::Canvas3DComponentBase>>;
using Canvas2DData = std::shared_ptr<Common::Canvas2DDataBase>;
using ImageData = Common::ImageBase;

#ifdef _WIN32
extern template class SyncDataStore2<std::string, int>; // test用
extern template class SyncDataStore2<ValueData, int>;
extern template class SyncDataStore2<TextData, int>;
extern template class SyncDataStore2<FuncData, int>;
extern template class SyncDataStore2<ViewData, int>;
extern template class SyncDataStore2<RobotModelData, int>;
extern template class SyncDataStore2<Canvas3DData, int>;
extern template class SyncDataStore2<Canvas2DData, int>;
extern template class SyncDataStore2<ImageData, Common::ImageReq>;
#endif

} // namespace Internal
WEBCFACE_NS_END
