#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <webcface/common/def.h>
#include <webcface/common/log.h>

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename T>
class WEBCFACE_DLL SyncDataStore1 {
    std::unordered_map<std::u8string, T> data_recv;
    std::unordered_map<std::u8string, bool> req;
    std::unordered_map<std::u8string, bool> req_send;

  public:
    std::u8string self_member_name;
    std::recursive_mutex mtx;

    explicit SyncDataStore1(const std::u8string &name)
        : self_member_name(name) {}

    //! リクエストを追加
    /*!
     * \return 追加した場合trueを返し、すでにリクエストされていた場合falseを返す
     */
    bool addReq(const std::u8string &member);

    //! リクエストを削除
    /*!
     * \return 削除した場合trueを返し、すでに削除されていた場合falseを返す
     */
    bool clearReq(const std::u8string &member);

    bool isSelf(std::u8string_view member) const {
        return member == self_member_name;
    }

    void setRecv(const std::u8string &member, const T &data);

    std::optional<T> getRecv(const std::u8string &member);
    //! req_sendを返し、req_sendをクリア
    std::unordered_map<std::u8string, bool> transferReq();
};

#ifdef _WIN32
extern template class SyncDataStore1<std::string>; // test用
extern template class SyncDataStore1<
    std::shared_ptr<std::vector<Common::LogLineData<>>>>;
extern template class SyncDataStore1<std::chrono::system_clock::time_point>;
#endif

} // namespace Internal
WEBCFACE_NS_END
