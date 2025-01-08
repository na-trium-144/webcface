#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/internal/client_internal.h"
#include "webcface/value.h"
#include "webcface/member.h"
#include <algorithm>
#include <cctype>

WEBCFACE_NS_BEGIN

ValueShape::ValueShape(const message::ValueShape &msg)
    : fixed_size(msg.size), is_fixed(msg.fixed) {}
ValueShape::operator message::ValueShape() const {
    return message::ValueShape{fixed_size, is_fixed};
}

Value::Value(const Field &base) : Field(base) {}

const Value &Value::request() const {
    auto data = dataLock();
    auto req = data->value_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(message::packSingle(
            message::Req<message::Value>{{}, member_, field_, req}));
    }
    return *this;
}

const Value &Value::set(std::vector<double> v, ValueShape shape) const {
    if (shape.is_fixed) {
        if (v.size() != shape.fixed_size) {
            throw std::invalid_argument("array size mismatch, expected: " +
                                        std::to_string(shape.fixed_size) +
                                        ", got: " + std::to_string(v.size()));
        }
    }
    auto data = setCheck();
    data->value_store.setSend(
        *this, std::make_shared<std::vector<double>>(std::move(v)));
    data->value_store.setEntry(*this, shape);
    std::shared_ptr<std::function<void(Value)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event = data->value_change_event[this->member_][this->field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}
const ValueElement<> &ValueElement<>::set(double v) const {
    Value parent = *this;
    auto pv = parent.tryGetVec();
    if (pv && index < pv->size()) {
        pv->at(index) = v;
        parent.set(*pv);
        return *this;
    } else {
        throw std::out_of_range("index out of range, current size: " +
                                std::to_string(pv ? pv->size() : 0) +
                                ", got index: " + std::to_string(index));
    }
}
const Value &Value::set(double v) const {
    set(std::vector<double>{v}, ValueShape{1, true});
    return *this;
}
const Value &Value::set(std::vector<double> v) const {
    return set(std::move(v), ValueShape{});
}


const Value &Value::onChange(std::function<void(Value)> callback) const {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->value_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Value)>>(std::move(callback));
    return *this;
}

const Value &Value::resize(std::size_t size) const {
    auto pv = this->tryGetVec();
    if (pv) {
        pv->resize(size);
    } else {
        pv.emplace(size);
    }
    this->set(*pv);
    return *this;
}
const Value &Value::push_back(double v) const {
    auto pv = this->tryGetVec();
    if (pv) {
        pv->push_back(v);
    } else {
        pv.emplace({v});
    }
    this->set(*pv);
    return *this;
}

std::optional<double> ValueElement<>::tryGet() const {
    Value parent = *this;
    auto pv = parent.tryGetVec();
    if (pv && index < pv->size()) {
        return pv->at(index);
    }
    return std::nullopt;
}
std::optional<double> Value::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v && v->size() >= 1) {
        return (*v)[0];
    }
    return std::nullopt;
}
std::optional<std::vector<double>> Value::tryGetVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return *v;
    } else {
        return std::nullopt;
    }
}
std::size_t Value::size() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    return v ? v->size() : 0;
}

void Value::assertSize(std::size_t size) const {
    if (dataLock()->isSelf(*this)) {
        resize(size);
    } else {
        auto v = dataLock()->value_store.getRecv(*this);
        if (v && v->size() != size) {
            throw std::runtime_error(
                "array size mismatch, expected: " + std::to_string(size) +
                ", actual data size is: " + std::to_string(v->size()));
        }
    }
}

std::chrono::system_clock::time_point Value::time() const {
    return member().syncTime();
}
const Value &Value::free() const {
    auto req = dataLock()->value_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

bool Value::exists() const {
    return dataLock()->value_store.getEntry(member_).count(field_);
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
