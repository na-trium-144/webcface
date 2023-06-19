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
    std::unordered_map<std::string, T> data_send;
    std::unordered_map<std::string, std::unordered_map<std::string, T>>
        data_recv;
    std::set<std::pair<std::string, std::string>> subsc, subsc_next;

  public:
    void set_send(const std::string &name, const T &data);
    void set_recv(const std::string &from, const std::string &name,
                  const T &data);
    std::optional<T> try_get_recv(const std::string &from,
                                  const std::string &name);
    std::unordered_map<std::string, T> transfer_send();
    std::set<std::pair<std::string, std::string>> transfer_subsc();
};

} // namespace WebCFace