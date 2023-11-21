#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <webcface/common/def.h>
#include <webcface/common/log.h>

namespace WebCFace::Internal {
template <typename T>
class SyncDataStore1 {
    std::mutex mtx;
    std::unordered_map<std::string, T> data_recv;
    std::unordered_map<std::string, bool> req;
    std::unordered_map<std::string, bool> req_send;
    std::string self_member_name;

  public:
    explicit SyncDataStore1(const std::string &name) : self_member_name(name) {}

    bool isSelf(const std::string &member) const {
        return member == self_member_name;
    }

    void setRecv(const std::string &member, const T &data);

    std::optional<T> getRecv(const std::string &member);
    //! req_sendを返し、req_sendをクリア
    std::unordered_map<std::string, bool> transferReq(bool is_first);
};

#ifdef _MSC_VER
extern template class SyncDataStore1<std::string>; // test用
extern template class SyncDataStore1<
    std::shared_ptr<std::vector<std::shared_ptr<Common::LogLine>>>>;
extern template class SyncDataStore1<std::chrono::system_clock::time_point>;
#endif

} // namespace WebCFace::Internal