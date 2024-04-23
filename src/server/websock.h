#pragma once
#include <spdlog/common.h>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Server {
extern WEBCFACE_DLL std::condition_variable server_ping_wait;
extern WEBCFACE_DLL std::mutex server_mtx;
extern WEBCFACE_DLL std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status;

WEBCFACE_DLL void serverSend(void *conn, const std::string &msg);
WEBCFACE_DLL void serverStop();
WEBCFACE_DLL void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level);
} // namespace Server
WEBCFACE_NS_END
