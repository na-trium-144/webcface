#pragma once
#include <spdlog/common.h>

namespace WebCFace {
namespace Server {
void serverStop();
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level);
} // namespace Server
} // namespace WebCFace
