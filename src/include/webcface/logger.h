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

template <typename CharT>
class WEBCFACE_DLL BasicLoggerBuf : public std::basic_streambuf<CharT> {
    using traits_type = std::basic_streambuf<CharT>::traits_type;
    using char_type = std::basic_streambuf<CharT>::char_type;
    using int_type = std::basic_streambuf<CharT>::int_type;

    static constexpr int buf_size = 1024;
    CharT buf[buf_size];
    // bufからあふれた分を入れる
    std::basic_string<CharT> overflow_buf;

    std::shared_ptr<spdlog::logger> logger;

    int sync() override;
    int overflow(int_type c) override;

  public:
    explicit BasicLoggerBuf(const std::shared_ptr<spdlog::logger> &logger);
    ~BasicLoggerBuf() = default;
};
extern template class WEBCFACE_IMPORT BasicLoggerBuf<char>;
extern template class WEBCFACE_IMPORT BasicLoggerBuf<wchar_t>;
using LoggerBuf = BasicLoggerBuf<char>;
using LoggerBufW = BasicLoggerBuf<wchar_t>;

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
