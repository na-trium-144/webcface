#include <webcface/log.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {

template class WEBCFACE_DLL EventTarget<Log, std::string>;

Log::Log(const Field &base)
    : Field(base), EventTarget<Log, std::string>(
                       &this->dataLock()->log_append_event, this->member_) {}


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
        return **v;
    } else {
        return std::nullopt;
    }
}

Log &Log::clear() {
    dataLock()->log_store->setRecv(member_,
                                   std::make_shared<std::vector<LogLine>>());
    return *this;
}

} // namespace WEBCFACE_NS
