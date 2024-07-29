#pragma once
#include <string>
#include <spdlog/logger.h>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace server {
std::string getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
std::string getTempDir(const std::shared_ptr<spdlog::logger> &logger);
} // namespace server
WEBCFACE_NS_END
