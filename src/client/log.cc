#include <webcface/log.h>
#include "client_internal.h"

namespace WebCFace {
Log::Log(const Field &base)
    : Field(base), EventTarget<Log, std::string>(
                       &this->dataLock()->log_append_event, this->member_) {}
std::optional<std::shared_ptr<std::vector<std::shared_ptr<LogLine>>>>
Log::getRaw() const {
    return dataLock()->log_store.getRecv(member_);
}
void Log::setRaw(
    const std::shared_ptr<std::vector<std::shared_ptr<LogLine>>> &raw) const {
    dataLock()->log_store.setRecv(member_, raw);
}

} // namespace WebCFace
