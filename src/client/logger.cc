#include <webcface/logger.h>
#include "client_internal.h"

namespace WEBCFACE_NS {
LoggerBuf::LoggerBuf(const std::shared_ptr<spdlog::logger> &logger)
    : std::streambuf(), logger(logger) {
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
        logger->info(message);
        overflow_buf = overflow_buf.substr(n + 1);
    }
    this->setp(buf, buf + sizeof(buf));
    return 0;
}

LoggerSink::LoggerSink(const std::shared_ptr<Internal::SyncDataStore1<
                           std::shared_ptr<std::vector<LogLine>>>> &log_store)
    : spdlog::sinks::base_sink<std::mutex>(), log_store(log_store) {}

void LoggerSink::sink_it_(const spdlog::details::log_msg &msg) {
    if (auto *buf_ptr = msg.payload.data()) {
        std::string log_text(buf_ptr, buf_ptr + msg.payload.size());

        if (log_text.size() > 0 && log_text.back() == '\n') {
            log_text.pop_back();
        }
        if (log_text.size() > 0 && log_text.back() == '\r') {
            log_text.pop_back();
        }
        {
            std::lock_guard lock(log_store->mtx);
            auto v = log_store->getRecv(log_store->self_member_name);
            // log_storeにはClientDataのコンストラクタで空vectorを入れてある
            (*v)->emplace_back(msg.level, msg.time, log_text);
        }
    }
}

} // namespace WEBCFACE_NS
