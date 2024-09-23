#pragma once
#include <mutex>
#include <unordered_map>
#include <optional>
#include <functional>
#include "webcface/field.h"
#include "webcface/encoding/val_adaptor.h"
#include "webcface/internal/func_internal.h"
#include "webcface/image_frame.h"
#include "webcface/component_canvas2d.h"
#include "webcface/component_canvas3d.h"
#include "webcface/component_view.h"
#include "webcface/robot_link.h"
#include "webcface/message/message.h"
#include "webcface/internal/component_internal.h"
#include "webcface/internal/robot_link_internal.h"
#include "webcface/log.h"

WEBCFACE_NS_BEGIN
namespace internal {
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
class SyncDataStore2 {
    /*!
     * \brief 次のsend時に送信するデータ。
     *
     */
    StrMap1<T> data_send;
    StrMap1<T> data_send_prev;
    /*!
     * \brief 送信済みデータ&受信済みデータ
     *
     * data_recv[member名][データ名] = 値
     *
     * ver1.11〜 setSend時には上書きされず、transferSendで上書きされる
     * それまでの間はgetRecvはdata_recvではなくdata_sendを優先的に読むようにする
     *
     */
    StrMap2<T> data_recv;
    /*!
     * \brief 受信済みのentry
     *
     * entry[member名] = {データ名のリスト}
     *
     */
    StrSet2 entry;
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
    void setSend(const SharedString &name, const T &data);
    void setSend(const FieldBase &base, const T &data);

    /*!
     * \brief 受信したデータをdata_recvにセット
     *
     */
    void setRecv(const SharedString &from, const SharedString &name,
                 const T &data);
    void setRecv(const FieldBase &base, const T &data);
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
    std::optional<T> getRecv(const SharedString &from,
                             const SharedString &name);
    std::optional<T> getRecv(const FieldBase &base);
    /*!
     * \brief data_recvからデータを削除, reqを消す
     *
     * \return reqを削除したらtrue, reqがすでに削除されてればfalse
     *
     */
    bool unsetRecv(const SharedString &from, const SharedString &name);
    bool unsetRecv(const FieldBase &base);

    /*!
     * \brief memberのentryをクリア
     *
     * ambiguousなので引数にFieldBaseは使わない (そもそも必要ない)
     */
    void clearEntry(const SharedString &from);
    /*!
     * \brief 受信したentryを追加
     *
     */
    void setEntry(const SharedString &from, const SharedString &e);

    /*!
     * \brief entryを取得
     *
     */
    StrSet1 getEntry(const SharedString &from);
    StrSet1 getEntry(const FieldBase &base);

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
    StrMap1<T> transferSend(bool is_first);
    StrMap1<T> getSendPrev(bool is_first);

    /*!
     * \brief req_sendを返し、req_sendをクリア
     *
     */
    StrMap2<unsigned int> transferReq();
};

struct Canvas2DDataBase {
    double width = 0, height = 0;
    std::vector<std::shared_ptr<Canvas2DComponentData>> components;
    Canvas2DDataBase() = default;
    Canvas2DDataBase(double width, double height)
        : width(width), height(height), components() {}
    Canvas2DDataBase(
        double width, double height,
        std::vector<std::shared_ptr<Canvas2DComponentData>> &&components)
        : width(width), height(height), components(std::move(components)) {}
};

using ValueData = std::shared_ptr<std::vector<double>>;
using TextData = std::shared_ptr<ValAdaptor>;
using FuncData = std::shared_ptr<FuncInfo>;
using ViewData =
    std::shared_ptr<std::vector<std::shared_ptr<internal::ViewComponentData>>>;
using RobotModelData =
    std::shared_ptr<std::vector<std::shared_ptr<internal::RobotLinkData>>>;
using Canvas3DData = std::shared_ptr<
    std::vector<std::shared_ptr<internal::Canvas3DComponentData>>>;
using Canvas2DData = std::shared_ptr<Canvas2DDataBase>;
using ImageData = ImageFrame;

struct LogData {
    std::deque<LogLineData> data;
    std::size_t sent_lines;

    std::vector<LogLineData> getDiff() {
        auto begin = data.cbegin() + static_cast<int>(sent_lines);
        auto end = data.cend();
        sent_lines = data.size();
        return std::vector<LogLineData>(begin, end);
    }
    std::vector<LogLineData> getAll() {
        sent_lines = data.size();
        return std::vector<LogLineData>(data.cbegin(), data.cend());
    }
};

#if WEBCFACE_SYSTEM_DLLEXPORT
extern template class SyncDataStore2<std::string, int>; // test用
extern template class SyncDataStore2<ValueData, int>;
extern template class SyncDataStore2<TextData, int>;
extern template class SyncDataStore2<FuncData, int>;
extern template class SyncDataStore2<ViewData, int>;
extern template class SyncDataStore2<RobotModelData, int>;
extern template class SyncDataStore2<Canvas3DData, int>;
extern template class SyncDataStore2<Canvas2DData, int>;
extern template class SyncDataStore2<ImageData, message::ImageReq>;
extern template class SyncDataStore2<std::shared_ptr<LogData>, int>;
#endif

} // namespace internal
WEBCFACE_NS_END
