#pragma once
#include <string>
#include <spdlog/logger.h>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Server {
std::string getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
std::string getTempDir(const std::shared_ptr<spdlog::logger> &logger);
} // namespace Server
WEBCFACE_NS_END
