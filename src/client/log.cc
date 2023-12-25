#include <webcface/log.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {
Log::Log(const Field &base)
    : Field(base), EventTarget<Log, std::string>(
                       &this->dataLock()->log_append_event, this->member_) {}

inline void addLogReq(const std::shared_ptr<Internal::ClientData> &data,
                      const std::string &member_) {
    auto req = data->log_store->addReq(member_);
    if (req) {
        data->message_queue->push(
            Message::packSingle(Message::LogReq{{}, member_}));
    }
}

void Log::onAppend() const { addLogReq(dataLock(), member_); }

std::optional<std::vector<LogLine>> Log::tryGet() const {
    auto v = dataLock()->log_store->getRecv(member_);
    addLogReq(dataLock(), member_);
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
