#include <stdexcept>
#include <webcface/logger.h>
#include "webcface/internal/client_internal.h"
#include <cstdio>

WEBCFACE_NS_BEGIN

template <typename CharT>
static void
writeLog(const std::shared_ptr<Internal::SyncDataStore1<
             std::shared_ptr<std::vector<LogLineData<>>>>> &log_store,
         std::basic_string_view<CharT> message, int level = 2,
         std::chrono::system_clock::time_point time =
             std::chrono::system_clock::now()) {
    std::lock_guard lock(log_store->mtx);
    auto v = log_store->getRecv(log_store->self_member_name);
    if (!v) [[unlikely]] {
        throw std::runtime_error("self log data is null");
    } else {
        // log_storeにはClientDataのコンストラクタで空vectorを入れてある
        (*v)->emplace_back(LogLineData<>{level, time, SharedString(message)});
    }
}

template <typename CharT>
BasicLoggerBuf<CharT>::BasicLoggerBuf(
    const std::shared_ptr<
        Internal::SyncDataStore1<std::shared_ptr<std::vector<LogLineData<>>>>>
        &log_store)
    : std::basic_streambuf<CharT>(), log_store(log_store) {
    this->setp(buf, buf + sizeof(buf));
}
template <typename CharT>
typename BasicLoggerBuf<CharT>::int_type
BasicLoggerBuf<CharT>::overflow(int_type c) {
    overflow_buf.append(buf, this->pptr() - this->pbase());
    this->setp(buf, buf + sizeof(buf));
    if (c != traits_type::eof()) {
        this->sputc(static_cast<char_type>(c));
    }
    return !traits_type::eof();
}
template <typename CharT>
int BasicLoggerBuf<CharT>::sync() {
    overflow_buf.append(buf, this->pptr() - this->pbase());
    while (true) {
        std::size_t n = overflow_buf.find_first_of('\n');
        if (n == std::basic_string<CharT>::npos) {
            break;
        }
        auto message = overflow_buf.substr(0, n);
        if (message.size() > 0 && message.back() == '\r') {
            message.pop_back();
        }
        writeLog<CharT>(log_store, message);
        if constexpr (std::is_same_v<CharT, char>) {
            std::fputs(message.c_str(), stderr);
        } else if constexpr (std::is_same_v<CharT, wchar_t>) {
            std::fputs(Encoding::toNarrow(message).c_str(), stderr);
        }
        std::fputc('\n', stderr);
        overflow_buf = overflow_buf.substr(n + 1);
    }
    this->setp(buf, buf + sizeof(buf));
    return 0;
}
template class WEBCFACE_DLL_INSTANCE_DEF BasicLoggerBuf<char>;
template class WEBCFACE_DLL_INSTANCE_DEF BasicLoggerBuf<wchar_t>;

LoggerSink::LoggerSink(
    const std::shared_ptr<
        Internal::SyncDataStore1<std::shared_ptr<std::vector<LogLineData<>>>>>
        &log_store)
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
        writeLog<char>(log_store, log_text, msg.level, msg.time);
    }
}

WEBCFACE_NS_END
