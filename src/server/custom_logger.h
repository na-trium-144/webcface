#pragma once
// crow.hの後にincludeすること

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <webcface/common/def.h>

namespace WEBCFACE_NS::Server {

class CustomLogger : public crow::ILogHandler {
    std::shared_ptr<spdlog::logger> logger;

  public:
    CustomLogger(std::shared_ptr<spdlog::logger> logger) : logger(logger) {}
    void log(std::string message, crow::LogLevel level) {
        logger->log(convertLevel(level), message);
    }
    spdlog::level::level_enum convertLevel(crow::LogLevel level) {
        switch (level) {
        case crow::LogLevel::Critical:
            return spdlog::level::critical;
        case crow::LogLevel::Error:
            return spdlog::level::err;
        case crow::LogLevel::Warning:
            return spdlog::level::warn;
        case crow::LogLevel::Info:
            return spdlog::level::info;
        case crow::LogLevel::Debug:
        default:
            return spdlog::level::debug;
        }
    }
};
} // namespace WEBCFACE_NS::Server
