#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <webcface/common/def.h>
#include <webcface/common/log.h>
#include <webcface/common/field_base.h>

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename T>
class SyncDataStore1 {
    std::unordered_map<MemberNameRef, T> data_recv;
    std::unordered_map<MemberNameRef, bool> req;
    std::unordered_map<MemberNameRef, bool> req_send;

  public:
    MemberNameRef self_member_name;
    std::recursive_mutex mtx;

    explicit SyncDataStore1(MemberNameRef name) : self_member_name(name) {}

    //! リクエストを追加
    /*!
     * \return 追加した場合trueを返し、すでにリクエストされていた場合falseを返す
     */
    bool addReq(MemberNameRef member);

    //! リクエストを削除
    /*!
     * \return 削除した場合trueを返し、すでに削除されていた場合falseを返す
     */
    bool clearReq(MemberNameRef member);

    bool isSelf(MemberNameRef member) const {
        return member == self_member_name;
    }

    void setRecv(MemberNameRef member, const T &data);

    std::optional<T> getRecv(MemberNameRef member);
    //! req_sendを返し、req_sendをクリア
    std::unordered_map<MemberNameRef, bool> transferReq();
};

#ifdef _MSC_VER
extern template class SyncDataStore1<std::string>; // test用
extern template class SyncDataStore1<
    std::shared_ptr<std::vector<Common::LogLine>>>;
extern template class SyncDataStore1<std::chrono::system_clock::time_point>;
#endif

} // namespace Internal
WEBCFACE_NS_END
