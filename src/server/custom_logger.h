#pragma once
// crow.hの後にincludeすること

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/common.h>

namespace WebCFace::Server {

class CustomLogger : public crow::ILogHandler {
    std::shared_ptr<spdlog::logger> logger;

  public:
    CustomLogger(std::shared_ptr<spdlog::logger> logger): logger(logger) {}
    void log(std::string message, crow::LogLevel level) {
        logger->log(convertLevel(level), message);
    }
    spdlog::level::level_enum convertLevel(crow::LogLevel level) {
        switch (level) {
        case crow::LogLevel::CRITICAL:
            return spdlog::level::critical;
        case crow::LogLevel::ERROR:
            return spdlog::level::err;
        case crow::LogLevel::WARNING:
            return spdlog::level::warn;
        case crow::LogLevel::INFO:
            return spdlog::level::info;
        case crow::LogLevel::DEBUG:
        default:
            return spdlog::level::debug;
        }
    }
};
}