#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <webcface/common/def.h>
#include <webcface/common/log.h>

namespace webcface::Internal {
template <typename T>
class SyncDataStore1 {
    std::unordered_map<std::string, T> data_recv;
    std::unordered_map<std::string, bool> req;
    std::unordered_map<std::string, bool> req_send;

  public:
    std::string self_member_name;
    std::recursive_mutex mtx;

    explicit SyncDataStore1(const std::string &name) : self_member_name(name) {}

    //! リクエストを追加
    /*!
     * \return 追加した場合trueを返し、すでにリクエストされていた場合falseを返す
     */
    bool addReq(const std::string &member);

    //! リクエストを削除
    /*!
     * \return 削除した場合trueを返し、すでに削除されていた場合falseを返す
     */
    bool clearReq(const std::string &member);

    bool isSelf(const std::string &member) const {
        return member == self_member_name;
    }

    void setRecv(const std::string &member, const T &data);

    std::optional<T> getRecv(const std::string &member);
    //! req_sendを返し、req_sendをクリア
    std::unordered_map<std::string, bool> transferReq();
};

#ifdef _MSC_VER
extern template class SyncDataStore1<std::string>; // test用
extern template class SyncDataStore1<
    std::shared_ptr<std::vector<Common::LogLine>>>;
extern template class SyncDataStore1<std::chrono::system_clock::time_point>;
#endif

} // namespace webcface::Internal