#pragma once
#include <spdlog/common.h>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>

namespace WebCFace {
namespace Server {
inline bool server_stop;
inline std::condition_variable server_ping_wait;
inline std::mutex server_mtx;
inline std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status;

void serverSend(void *conn, const std::string &msg);
void serverStop();
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level);
} // namespace Server
} // namespace WebCFace
