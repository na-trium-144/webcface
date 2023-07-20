#pragma once
#include <unordered_map>
#include <set>
#include <mutex>
#include <optional>

namespace WebCFace {

template <typename T>
class SyncDataStore {
  private:
    std::mutex mtx;
    // 次のsendで送信するデータ
    std::unordered_map<std::string, T> data_send;
    // 送信済みデータ, 受信済みデータ
    std::unordered_map<std::string, std::unordered_map<std::string, T>>
        data_recv;
    // 受信済みentry
    std::unordered_map<std::string, std::vector<std::string>> entry;
    // req (subsc)
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> req,
        req_send;

  public:
    void setSend(const std::string &name, const T &data);
    void setRecv(const std::string &from, const std::string &name,
                 const T &data);
    // return recv or set req
    std::optional<T> getRecv(const std::string &from, const std::string &name);
    void unsetRecv(const std::string &from, const std::string &name);

    void setEntry(const std::string &from, const std::vector<std::string> &e);
    std::vector<std::string> getEntry(const std::string &from);
    std::vector<std::string> getEntries();

    std::unordered_map<std::string, T> transferSend();
    std::unordered_map<std::string, std::unordered_map<std::string, bool>>
    transferReq();
};

} // namespace WebCFace