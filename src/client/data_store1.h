#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <optional>

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

} // namespace WebCFace::Internal