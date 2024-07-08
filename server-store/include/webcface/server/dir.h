#pragma once
#include <string>
#include <spdlog/logger.h>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace server {
WEBCFACE_DLL std::string
getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
WEBCFACE_DLL std::string
getTempDir(const std::shared_ptr<spdlog::logger> &logger);
} // namespace server
WEBCFACE_NS_END
