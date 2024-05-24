#include <webcface/log.h>
#include "client_internal.h"
#include "../message/message.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL EventTarget<Log>;

Log::Log(const Field &base) : Field(base), EventTarget<Log>() {
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(this->dataLock()->log_append_event[this->member_]);
}

void Log::request() const {
    auto data = dataLock();
    auto req = data->log_store->addReq(member_);
    if (req) {
        data->message_queue->push(
            Message::packSingle(Message::LogReq{{}, member_}));
    }
}

void Log::onAppend() const { request(); }

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
