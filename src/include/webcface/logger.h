#pragma once
#include <streambuf>
#include <memory>
#include <string>
#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "common/queue.h"
#include "common/log.h"
#include "common/def.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}

class LoggerBuf : public std::streambuf {
    static constexpr int buf_size = 1024;
    char buf[buf_size];
    // bufからあふれた分を入れる
    std::string overflow_buf;

    std::shared_ptr<spdlog::logger> logger;

    WEBCFACE_DLL int sync() override;
    WEBCFACE_DLL int overflow(int c) override;

  public:
    WEBCFACE_DLL explicit LoggerBuf(
        const std::shared_ptr<spdlog::logger> &logger);
    LoggerBuf(const LoggerBuf &) = delete;
    LoggerBuf &operator=(const LoggerBuf &) = delete;
};

namespace Internal {
template <typename T>
class SyncDataStore1;
}

class LoggerSink : public spdlog::sinks::base_sink<std::mutex> {
    std::shared_ptr<
        Internal::SyncDataStore1<std::shared_ptr<std::vector<LogLine>>>>
        log_store;

  protected:
    WEBCFACE_DLL void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override {}

  public:
    WEBCFACE_DLL explicit LoggerSink(
        const std::shared_ptr<
            Internal::SyncDataStore1<std::shared_ptr<std::vector<LogLine>>>>
            &log_store);
    void set_pattern_(const std::string &) override {}
    void set_formatter_(std::unique_ptr<spdlog::formatter>) override {}
};

} // namespace WEBCFACE_NS
