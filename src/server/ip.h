#pragma once
#include <webcface/common/def.h>
#include <vector>
#include <string>
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/logger.h>

namespace WEBCFACE_NS {
namespace Server {
std::vector<std::string>
getIpAddresses(const std::shared_ptr<spdlog::logger> &logger);
}
} // namespace WEBCFACE_NS
