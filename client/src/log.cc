#include <webcface/log.h>
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN

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

Log::Log(const Field &base) : Field(base) {}

Log &Log::onAppend(std::function<void(Log)> callback) {
    this->request();
    std::lock_guard lock(this->dataLock()->event_m);
    this->dataLock()->log_append_event[this->member_] = std::move(callback);
    return *this;
}

void Log::request() const {
    auto data = dataLock();
    auto req = data->log_store->addReq(member_);
    if (req) {
        data->message_push(message::packSingle(message::LogReq{{}, member_}));
    }
}

std::optional<std::vector<LogLine>> Log::tryGet() const {
    request();
    auto v = dataLock()->log_store->getRecv(member_);
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
    auto v = dataLock()->log_store->getRecv(member_);
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
    dataLock()->log_store->setRecv(
        member_, std::make_shared<std::vector<LogLineData<>>>());
    return *this;
}

WEBCFACE_NS_END
