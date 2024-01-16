#pragma once
#include <webcface/common/def.h>
#include <vector>
#include <string>
#include <spdlog/logger.h>

namespace WEBCFACE_NS {
namespace Server {
std::vector<std::string>
getIpAddresses(const std::shared_ptr<spdlog::logger> &logger);
}
} // namespace WEBCFACE_NS
