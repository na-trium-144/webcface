#pragma once
#include <webcface/common/def.h>
#include <vector>
#include <string>
#include <spdlog/logger.h>

WEBCFACE_NS_BEGIN
namespace Server {
std::vector<std::string>
getIpAddresses(const std::shared_ptr<spdlog::logger> &logger);
}
WEBCFACE_NS_END
