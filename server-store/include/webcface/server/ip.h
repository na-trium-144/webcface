#pragma once
#include "webcface/common/def.h"
#include <vector>
#include <string>
#include <memory>
#include <spdlog/logger.h>

WEBCFACE_NS_BEGIN
namespace server {
std::vector<std::string> WEBCFACE_CALL
getIpAddresses(const std::shared_ptr<spdlog::logger> &logger);

std::string WEBCFACE_CALL getHostName(const std::shared_ptr<spdlog::logger> &logger);
}
WEBCFACE_NS_END
