#pragma once
#include <mutex>
#include <optional>
#include "webcface/log.h"
#include "webcface/encoding/encoding.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename T>
class SyncDataStore1 {
    StrMap1<T> data_recv;
    StrMap1<bool> req;
    StrMap1<bool> req_send;
    /*!
     * \brief 受信済みのentry
     *
     * entry[member名] = {データ名のリスト}
     *
     */
    StrSet1 entry;

  public:
    SharedString self_member_name;
    std::recursive_mutex mtx;

    explicit SyncDataStore1(const SharedString &name);

    //! リクエストを追加
    /*!
     * \return 追加した場合trueを返し、すでにリクエストされていた場合falseを返す
     */
    bool addReq(const SharedString &member);

    //! リクエストを削除
    /*!
     * \return 削除した場合trueを返し、すでに削除されていた場合falseを返す
     */
    bool clearReq(const SharedString &member);

    bool isSelf(const SharedString &member) const;

    void setRecv(const SharedString &member, const T &data);

    /*!
     * \brief memberのentryをクリア
     */
    void clearEntry(const SharedString &from);
    /*!
     * \brief 受信したentryを追加
     */
    void setEntry(const SharedString &from);

    /*!
     * \brief entryを取得
     */
    bool getEntry(const SharedString &from);
    bool getEntry(const FieldBase &base);

    std::optional<T> getRecv(const SharedString &member);
    //! req_sendを返し、req_sendをクリア
    StrMap1<bool> transferReq();
};

#if WEBCFACE_SYSTEM_DLLEXPORT
extern template class SyncDataStore1<std::string>; // test用
extern template class SyncDataStore1<std::shared_ptr<std::deque<LogLineData>>>;
extern template class SyncDataStore1<std::chrono::system_clock::time_point>;
#endif

} // namespace internal
WEBCFACE_NS_END
