#pragma once
#include <streambuf>
#include <memory>
#include <string>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "common/queue.h"
#include "common/log.h"
#include "common/def.h"

namespace WebCFace {
struct ClientData;

class WEBCFACE_DLL LoggerBuf : public std::streambuf {
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

class WEBCFACE_DLL LoggerSink : public spdlog::sinks::base_sink<std::mutex>,
                   public Queue<std::shared_ptr<LogLine>> {
  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override {}

  public:
    explicit LoggerSink();
    void set_pattern_(const std::string &) override {}
    void set_formatter_(std::unique_ptr<spdlog::formatter>) override {}
};

//! stderrに出力するsink
//! 全clientで共通
//! loggerからstderrに流すのを止めたい時などはこれのset_levelなどを使う
WEBCFACE_DLL extern std::shared_ptr<spdlog::sinks::stderr_color_sink_mt>
    stderr_sink;

//! webcfaceのログ出力レベルを設定できます
WEBCFACE_DLL extern spdlog::level::level_enum logger_internal_level;

} // namespace WebCFace