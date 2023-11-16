#include <webcface/text.h>
#include "client_internal.h"

namespace WebCFace {
Text &Text::set(const std::shared_ptr<std::string> &v) {
    setCheck();
    dataLock()->text_store.setSend(*this, v);
    this->triggerEvent(*this);
    return *this;
}
std::optional<std::shared_ptr<std::string>> Text::getRaw() const {
    return dataLock()->text_store.getRecv(*this);
}
std::optional<Text::Dict> Text::getRawRecurse() const {
    return dataLock()->text_store.getRecvRecurse(*this);
}
Text::Text(const Field &base)
    : Field(base), EventTarget<Text>(&this->dataLock()->text_change_event,
                                     *this) {}
std::chrono::system_clock::time_point Text::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
Text &Text::free() {
    dataLock()->text_store.unsetRecv(*this);
    return *this;
}

} // namespace WebCFace