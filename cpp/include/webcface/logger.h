#pragma once
#include <streambuf>
#include <memory>
#include <string>
#include <spdlog/sinks/base_sink.h>
#include "common/queue.h"

namespace WebCFace {
class ClientData;

struct LogLine {
    int level;
    std::string message;
};

class LoggerBuf : public std::streambuf {
    static constexpr int buf_size = 1024;
    char buf[buf_size];
    // bufからあふれた分を入れる
    std::string overflow_buf;

    std::weak_ptr<ClientData> data_w;

    int sync() override;
    int overflow(int c) override;

  public:
    explicit LoggerBuf(const std::weak_ptr<ClientData> &data_w);
    LoggerBuf(const LoggerBuf &) = delete;
    LoggerBuf &operator=(const LoggerBuf &) = delete;
};

class LoggerSink : public spdlog::sinks::base_sink<std::mutex>,
                   public Queue<LogLine> {
  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override {}

  public:
    explicit LoggerSink();
    void set_pattern_(const std::string &) override {}
    void set_formatter_(std::unique_ptr<spdlog::formatter>) override {}
};

} // namespace WebCFace