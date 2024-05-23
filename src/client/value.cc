#include "client_internal.h"
#include <webcface/value.h>
#include <webcface/member.h>
#include "../message/message.h"
#include <algorithm>
#include <cctype>

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL EventTarget<Value>;

Value::Value(const Field &base) : Field(base), EventTarget<Value>() {
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(this->dataLock()->value_change_event[*this]);
}

void Value::request() const {
    auto data = dataLock();
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
    auto last_name = this->lastName();
    auto parent = this->parent();
    if (std::all_of(last_name.cbegin(), last_name.cend(),
                    [](unsigned char c) { return std::isdigit(c); }) &&
        v.size() == 1) {
        std::size_t index = std::stoi(std::string(last_name));
        auto pv = parent.tryGetVec();
        if (pv && index < pv->size() + 10) { // てきとう
            while (pv->size() <= index) {
                pv->push_back(0);
            }
            pv->at(index) = v;
            parent.set(*pv);
            return *this;
        }
    }
    setCheck()->value_store.setSend(*this,
                                    std::make_shared<VectorOpt<double>>(v));
    this->triggerEvent(*this);
    return *this;
}
Value &Value::resize(std::size_t size) {
    auto pv = this->tryGetVec();
    if (pv) {
        pv->resize(size);
    } else {
        pv.emplace(size);
    }
    this->set(*pv);
    return *this;
}
Value &Value::push_back(double v) {
    auto pv = this->tryGetVec();
    if (pv) {
        pv->push_back(v);
    } else {
        pv.emplace({v});
    }
    this->set(*pv);
    return *this;
}

void Value::onAppend() const { request(); }

std::optional<double> Value::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return **v;
    }
    auto last_name = lastName();
    if (std::all_of(last_name.cbegin(), last_name.cend(),
                    [](unsigned char c) { return std::isdigit(c); })) {
        std::size_t index = std::stoi(std::string(last_name));
        auto pv = parent().tryGetVec();
        if (pv && index < pv->size()) {
            return pv->at(index);
        }
    }
    return std::nullopt;
}
std::optional<std::vector<double>> Value::tryGetVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return **v;
    } else {
        return std::nullopt;
    }
}
std::optional<Value::Dict> Value::tryGetRecurse() const {
    request();
    return dataLock()->value_store.getRecvRecurse(
        *this, [this](const std::u8string &subfield) {
            Value(this->Field::child(subfield)).request();
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

std::ostream &operator<<(std::ostream &os, const Value &data) {
    auto v = data.tryGetVec();
    if (v) {
        os << (v->size() > 0 ? v->at(0) : 0.0);
        for (std::size_t i = 1; i < v->size(); i++) {
            os << ", " << v->at(i);
        }
    } else {
        os << "null";
    }
    return os;
}

WEBCFACE_NS_END
