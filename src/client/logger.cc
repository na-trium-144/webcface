#include <webcface/logger.h>
#include "client_internal.h"

namespace WebCFace {
LoggerBuf::LoggerBuf(const std::weak_ptr<Internal::ClientData> &data_w)
    : std::streambuf(), data_w(data_w) {
    this->setp(buf, buf + sizeof(buf));
}
int LoggerBuf::overflow(int c) {
    overflow_buf += std::string(buf, this->pptr() - this->pbase());
    this->setp(buf, buf + sizeof(buf));
    this->sputc(c);
    return 0;
}
int LoggerBuf::sync() {
    overflow_buf += std::string(buf, this->pptr() - this->pbase());
    while (true) {
        std::size_t n = overflow_buf.find_first_of('\n');
        if (n == std::string::npos) {
            break;
        }
        std::string message = overflow_buf.substr(0, n);
        if (message.size() > 0 && message.back() == '\r') {
            message.pop_back();
        }
        data_w.lock()->logger->info(message);
        overflow_buf = overflow_buf.substr(n + 1);
    }
    this->setp(buf, buf + sizeof(buf));
    return 0;
}

LoggerSink::LoggerSink() : spdlog::sinks::base_sink<std::mutex>() {}

void LoggerSink::sink_it_(const spdlog::details::log_msg &msg) {
    if (auto *buf_ptr = msg.payload.data()) {
        std::string log_text(buf_ptr, buf_ptr + msg.payload.size());

        if (log_text.size() > 0 && log_text.back() == '\n') {
            log_text.pop_back();
        }
        if (log_text.size() > 0 && log_text.back() == '\r') {
            log_text.pop_back();
        }
        this->push(std::make_shared<LogLine>(msg.level, msg.time, log_text));
    }
}

std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> stderr_sink =
    std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

spdlog::level::level_enum logger_internal_level = spdlog::level::info;

} // namespace WebCFace