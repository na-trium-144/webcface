#include "client_internal.h"
#include <webcface/value.h>

namespace webcface {
Value::Value(const Field &base)
    : Field(base), EventTarget<Value>(&this->dataLock()->value_change_event,
                                      *this) {}
Value &Value::set(const std::shared_ptr<VectorOpt<double>> &v) {
    setCheck();
    dataLock()->value_store.setSend(*this, v);
    this->triggerEvent(*this);
    return *this;
}
std::optional<std::shared_ptr<VectorOpt<double>>> Value::getRaw() const {
    return dataLock()->value_store.getRecv(*this);
}
std::optional<Value::Dict> Value::getRawRecurse() const {
    return dataLock()->value_store.getRecvRecurse(*this);
}
std::chrono::system_clock::time_point Value::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
Value &Value::free() {
    dataLock()->value_store.unsetRecv(*this);
    return *this;
}


} // namespace webcface