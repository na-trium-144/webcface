#include "client_internal.h"
#include <webcface/value.h>
#include <webcface/member.h>
#include "../message/message.h"

namespace WEBCFACE_NS {
Value::Value(const Field &base)
    : Field(base), EventTarget<Value>(&this->dataLock()->value_change_event,
                                      *this) {}

inline void addValueReq(const std::shared_ptr<Internal::ClientData> &data,
                        const std::string &member_, const std::string &field_) {
    auto req = data->value_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Value>{{}, member_, field_, req}));
    }
}

Value &Value::set(const Value::Dict &v) {
    if (v.hasValue()) {
        setCheck()->value_store.setSend(*this, v.getRaw());
        this->triggerEvent(*this);
    } else {
        for (const auto &it : v.getChildren()) {
            child(it.first).set(it.second);
        }
    }
    return *this;
}
Value &Value::set(const VectorOpt<double> &v) {
    setCheck()->value_store.setSend(*this,
                                    std::make_shared<VectorOpt<double>>(v));
    this->triggerEvent(*this);
    return *this;
}

void Value::onAppend() const { addValueReq(dataLock(), member_, field_); }

std::optional<double> Value::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    addValueReq(dataLock(), member_, field_);
    if (v) {
        return **v;
    } else {
        return std::nullopt;
    }
}
std::optional<std::vector<double>> Value::tryGetVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    addValueReq(dataLock(), member_, field_);
    if (v) {
        return **v;
    } else {
        return std::nullopt;
    }
}
std::optional<Value::Dict> Value::tryGetRecurse() const {
    addValueReq(dataLock(), member_, field_);
    return dataLock()->value_store.getRecvRecurse(
        *this, [this](const std::string &subfield) {
            addValueReq(dataLock(), member_, subfield);
        });
}
std::chrono::system_clock::time_point Value::time() const {
    return member().syncTime();
}
Value &Value::free() {
    auto req = dataLock()->value_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}


} // namespace WEBCFACE_NS
