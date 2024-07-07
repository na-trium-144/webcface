#include <webcface/log.h>
#include <webcface/internal/logger.h>
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"
#include "webcface/internal/event_target_impl.h"

WEBCFACE_NS_BEGIN

template <typename CharT>
LogLineData<CharT>::LogLineData(int level,
                                std::chrono::system_clock::time_point time,
                                const SharedString &message)
    : level_(level), time_(time), message_(message) {}

template <typename CharT>
LogLineData<CharT>::LogLineData(const message::LogLine &m)
    : LogLineData(m.level_,
                  std::chrono::system_clock::time_point(
                      std::chrono::milliseconds(m.time_ms)),
                  m.message_) {}
template <typename CharT>
message::LogLine LogLineData<CharT>::toMessage() const {
    return message::LogLine{
        level_,
        static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                time_.time_since_epoch())
                .count()),
        message_};
}

template class WEBCFACE_DLL_INSTANCE_DEF LogLineData<char8_t>;
template class WEBCFACE_DLL_INSTANCE_DEF LogLineData<char>;
template class WEBCFACE_DLL_INSTANCE_DEF LogLineData<wchar_t>;
template class WEBCFACE_DLL_INSTANCE_DEF EventTarget<Log>;


static void writeLog(internal::ClientData *data_p, LogLineData<> &&ll) {
    std::lock_guard lock(data_p->log_store.mtx);
    auto v = data_p->log_store.getRecv(data_p->self_member_name);
    if (!v) [[unlikely]] {
        throw std::runtime_error("self log data is null");
    } else {
        // log_storeにはClientDataのコンストラクタで空vectorを入れてある
        (*v)->emplace_back(std::move(ll));
    }
}

template <typename CharT>
BasicLoggerBuf<CharT>::BasicLoggerBuf(internal::ClientData *data_p)
    : std::basic_streambuf<CharT>(), data_p(data_p) {
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
        writeLog(data_p,
                 {2, std::chrono::system_clock::now(), SharedString(message)});
        if constexpr (std::is_same_v<CharT, char>) {
            std::fputs(message.c_str(), stderr);
        } else if constexpr (std::is_same_v<CharT, wchar_t>) {
            std::fputs(encoding::toNarrow(message).c_str(), stderr);
        }
        std::fputc('\n', stderr);
        overflow_buf = overflow_buf.substr(n + 1);
    }
    this->setp(buf, buf + sizeof(buf));
    return 0;
}
template class WEBCFACE_DLL_INSTANCE_DEF BasicLoggerBuf<char>;
template class WEBCFACE_DLL_INSTANCE_DEF BasicLoggerBuf<wchar_t>;

Log::Log(const Field &base) : Field(base), EventTarget<Log>() {
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(this->dataLock()->log_append_event[this->member_]);
}

void Log::request() const {
    auto data = dataLock();
    auto req = data->log_store.addReq(member_);
    if (req) {
        data->message_push(message::packSingle(message::LogReq{{}, member_}));
    }
}

void Log::onAppend() const { request(); }

std::optional<std::vector<LogLine>> Log::tryGet() const {
    request();
    auto v = dataLock()->log_store.getRecv(member_);
    if (v) {
        std::vector<LogLine> log_s;
        log_s.reserve((*v)->size());
        for (const auto &l : **v) {
            log_s.emplace_back(l);
        }
        return log_s;
    } else {
        return std::nullopt;
    }
}
std::optional<std::vector<LogLineW>> Log::tryGetW() const {
    request();
    auto v = dataLock()->log_store.getRecv(member_);
    if (v) {
        std::vector<LogLineW> log_s;
        log_s.reserve((*v)->size());
        for (const auto &l : **v) {
            log_s.emplace_back(l);
        }
        return log_s;
    } else {
        return std::nullopt;
    }
}

Log &Log::clear() {
    dataLock()->log_store.setRecv(
        member_, std::make_shared<std::vector<LogLineData<>>>());
    return *this;
}

Log &Log::append(LogLineData<> &&ll) {
    writeLog(setCheck().get(), std::move(ll));
    return *this;
}

WEBCFACE_NS_END
