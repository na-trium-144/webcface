#pragma once
#include <string>
#include <spdlog/logger.h>
#include <webcface/common/def.h>

namespace WEBCFACE_NS {
namespace Server {
std::string getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
std::string getTempDir(const std::shared_ptr<spdlog::logger> &logger);
} // namespace Server
} // namespace WEBCFACE_NS
