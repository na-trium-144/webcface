#pragma once
#include <string>
#include <spdlog/logger.h>

namespace WebCFace{
namespace Server{
std::string getStaticDir(const std::shared_ptr<spdlog::logger> &logger);
std::string getTempDir(const std::shared_ptr<spdlog::logger> &logger);
}
}