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
WEBCFACE_DLL std::vector<std::string> WEBCFACE_CALL
getIpAddresses(const std::shared_ptr<spdlog::logger> &logger);

WEBCFACE_DLL std::string WEBCFACE_CALL getHostName(const std::shared_ptr<spdlog::logger> &logger);
}
WEBCFACE_NS_END
