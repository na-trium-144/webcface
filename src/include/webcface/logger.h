#pragma once
#include <streambuf>
#include <memory>
#include <string>
#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "common/queue.h"
#include "common/log.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
}

class WEBCFACE_DLL LoggerBuf : public std::streambuf {
    static constexpr int buf_size = 1024;
    char buf[buf_size];
    // bufからあふれた分を入れる
    std::string overflow_buf;

    std::shared_ptr<spdlog::logger> logger;

    int sync() override;
    int overflow(int c) override;

  public:
    explicit LoggerBuf(const std::shared_ptr<spdlog::logger> &logger);
    LoggerBuf(const LoggerBuf &) = delete;
    LoggerBuf &operator=(const LoggerBuf &) = delete;
};

namespace Internal {
template <typename T>
class SyncDataStore1;
}

class WEBCFACE_DLL LoggerSink : public spdlog::sinks::base_sink<std::mutex> {
    std::shared_ptr<
        Internal::SyncDataStore1<std::shared_ptr<std::vector<LogLineData<>>>>>
        log_store;

  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override {}

  public:
    explicit LoggerSink(
        const std::shared_ptr<Internal::SyncDataStore1<
            std::shared_ptr<std::vector<LogLineData<>>>>> &log_store);
    void set_pattern_(const std::string &) override {}
    void set_formatter_(std::unique_ptr<spdlog::formatter>) override {}
};

WEBCFACE_NS_END
