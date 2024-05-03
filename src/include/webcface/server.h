#pragma once
#include <webcface/common/def.h>
#include <spdlog/common.h>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <future>
#include <vector>

WEBCFACE_NS_BEGIN
namespace Server {
struct ServerStorage;
struct MemberData;

using wsConnPtr = void *;

class WEBCFACE_DLL Server {
    std::atomic<bool> server_stop;
    std::mutex server_mtx;
    std::vector<void *> apps;
    std::vector<std::shared_future<void>> apps_running;

    void pingThreadMain();

    void send(wsConnPtr conn, const std::string &msg);

  public:
    friend MemberData;
    std::condition_variable server_ping_wait;
    std::unique_ptr<ServerStorage> store;

  private:
    std::thread ping_thread; // storeよりも後ろ

  public:
    Server(int port, const spdlog::sink_ptr &sink,
           spdlog::level::level_enum level, int keep_log = 1000);
    ~Server();
    void join();

    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;
};
} // namespace Server
WEBCFACE_NS_END
