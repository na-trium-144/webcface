#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/config.h"
#endif
#include <vector>
#include <string>
#include <memory>
#include <spdlog/logger.h>

WEBCFACE_NS_BEGIN
namespace server {
std::vector<std::string>
getIpAddresses(const std::shared_ptr<spdlog::logger> &logger);

std::string getHostName(const std::shared_ptr<spdlog::logger> &logger);
} // namespace server
WEBCFACE_NS_END
