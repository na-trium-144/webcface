#pragma once
#include <string>
#include <spdlog/logger.h>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/config.h"
#endif

WEBCFACE_NS_BEGIN
namespace server {
WEBCFACE_DLL std::string WEBCFACE_CALL
getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
WEBCFACE_DLL std::string WEBCFACE_CALL
getTempDir(const std::shared_ptr<spdlog::logger> &logger);
} // namespace server
WEBCFACE_NS_END
