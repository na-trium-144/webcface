#pragma once
#include <string>
#include <spdlog/logger.h>
#include "webcface/common/def.h"

WEBCFACE_NS_BEGIN
namespace server {
std::string WEBCFACE_CALL
getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
std::string WEBCFACE_CALL
getTempDir(const std::shared_ptr<spdlog::logger> &logger);
} // namespace server
WEBCFACE_NS_END
