#include "webcface/common/internal/message/pack.h"
#include "webcface/log.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/internal/logger.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/log_history.h"

WEBCFACE_NS_BEGIN

std::atomic<int> internal::log_keep_lines = 1000;

void Log::keepLines(int n) { internal::log_keep_lines.store(n); }

LogLineData::LogLineData(int level, std::chrono::system_clock::time_point time,
                         const SharedString &message)
    : level_(level), time_(time), message_(message) {}

LogLineData::LogLineData(const message::LogLine &m)
    : LogLineData(m.level_,
                  std::chrono::system_clock::time_point(
                      std::chrono::milliseconds(m.time_ms)),
                  m.message_) {}
message::LogLine LogLineData::toMessage() const {
    return message::LogLine{
        level_,
        static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                time_.time_since_epoch())
                .count()),
        message_};
}

/// \private
static void writeLog(internal::ClientData *data_p, const SharedString &field,
                     LogLineData &&ll) {
    std::lock_guard lock(data_p->log_store.mtx);
    auto v = data_p->log_store.getRecv(data_p->self_member_name, field);
    if (!v) {
        v = std::make_shared<internal::LogHistory>();
    }
    v->data.emplace_back(std::move(ll));
    data_p->log_store.setSend(field, v);
}

template <typename CharT>
BasicLoggerBuf<CharT>::BasicLoggerBuf(internal::ClientData *data_p,
                                      const SharedString &field)
    : std::basic_streambuf<CharT>(), data_p(data_p), field(field) {
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
        writeLog(data_p, field,
                 {2, std::chrono::system_clock::now(),
                  SharedString::encode(message)});
        if constexpr (std::is_same_v<CharT, char>) {
            std::fputs(message.c_str(), stderr);
        } else if constexpr (std::is_same_v<CharT, wchar_t>) {
            std::fputs(toNarrow(message).c_str(), stderr);
        }
        std::fputc('\n', stderr);
        overflow_buf = overflow_buf.substr(n + 1);
    }
    this->setp(buf, buf + sizeof(buf));
    return 0;
}
template class BasicLoggerBuf<char>;
template class BasicLoggerBuf<wchar_t>;

Log::Log(const Field &base) : Field(base) {}

const Log &Log::onChange(std::function<void(Log)> callback) const {
    this->request();
    std::lock_guard lock(this->dataLock()->event_m);
    this->dataLock()->log_append_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Log)>>(std::move(callback));
    return *this;
}

const Log &Log::request() const {
    auto data = dataLock();
    auto req = data->log_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(
            message::Req<message::Log>{{}, member_, field_, req});
    }
    return *this;
}

std::optional<std::vector<LogLine>> Log::tryGet() const {
    request();
    auto v = dataLock()->log_store.getRecv(member_, field_);
    if (v) {
        std::vector<LogLine> log_s;
        log_s.reserve(v->data.size());
        for (const auto &l : v->data) {
            log_s.emplace_back(l);
        }
        return log_s;
    } else {
        return std::nullopt;
    }
}
std::optional<std::vector<LogLineW>> Log::tryGetW() const {
    request();
    auto v = dataLock()->log_store.getRecv(member_, field_);
    if (v) {
        std::vector<LogLineW> log_s;
        log_s.reserve(v->data.size());
        for (const auto &l : v->data) {
            log_s.emplace_back(l);
        }
        return log_s;
    } else {
        return std::nullopt;
    }
}

bool Log::exists() const {
    return dataLock()->log_store.getEntry(member_).count(field_);
}

const Log &Log::clear() const {
    dataLock()->log_store.setRecv(member_, field_,
                                  std::make_shared<internal::LogHistory>());
    return *this;
}

const Log &Log::append(LogLineData &&ll) const {
    writeLog(setCheck().get(), field_, std::move(ll));
    return *this;
}

WEBCFACE_NS_END
