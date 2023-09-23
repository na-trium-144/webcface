#pragma once
#include <spdlog/common.h>
#include <mutex>
#include <condition_variable>

namespace WebCFace {
namespace Server {
inline bool server_stop;
inline std::condition_variable server_stop_cond;
inline std::mutex server_mtx;
void serverStop();
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level);
} // namespace Server
} // namespace WebCFace
