#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <webcface/common/def.h>
#include <webcface/common/log.h>
#include <webcface/encoding.h>

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename T>
class WEBCFACE_TEMPLATE SyncDataStore1 {
    StrMap1<T> data_recv;
    StrMap1<bool> req;
    StrMap1<bool> req_send;

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

    std::optional<T> getRecv(const SharedString &member);
    //! req_sendを返し、req_sendをクリア
    StrMap1<bool> transferReq();
};

#ifdef _WIN32
extern template class WEBCFACE_DLL_INSTANCE_DECL
    SyncDataStore1<std::string>; // test用
extern template class WEBCFACE_DLL_INSTANCE_DECL
    SyncDataStore1<std::shared_ptr<std::vector<Common::LogLineData<>>>>;
extern template class WEBCFACE_DLL_INSTANCE_DECL
    SyncDataStore1<std::chrono::system_clock::time_point>;
#endif

} // namespace Internal
WEBCFACE_NS_END
