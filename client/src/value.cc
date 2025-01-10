#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/internal/client_internal.h"
#include "webcface/value.h"
#include "webcface/member.h"
#include <algorithm>
#include <cctype>
#include <numeric>

WEBCFACE_NS_BEGIN

ValueShape::ValueShape(const message::ValueShape &msg)
    : fixed_shape(msg.shape), is_fixed(msg.fixed) {}
ValueShape::operator message::ValueShape() const {
    return message::ValueShape{fixed_shape, is_fixed};
}

Value::Value(const Field &base) : Field(base) {}

const Value &Value::request() const {
    auto data = dataLock();
    auto req = data->value_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(
            message::Req<message::Value>{{}, member_, field_, req});
    }
    return *this;
}

const Value &Value::setImpl(std::vector<double> v,
                            const ValueShape &shape) const {
    std::size_t size = std::accumulate(
        shape.fixed_shape.begin(), shape.fixed_shape.end(),
        static_cast<std::size_t>(1), [](auto a, auto b) { return a * b; });
    if (shape.is_fixed) {
        if (v.size() != size) {
            throw std::invalid_argument(
                "array size mismatch, expected: " + std::to_string(size) +
                ", got: " + std::to_string(v.size()));
        }
    } else {
        if (v.size() % size != 0) {
            throw std::invalid_argument(
                "array size mismatch, expected multiple of: " +
                std::to_string(size) + ", got: " + std::to_string(v.size()));
        }
    }
    auto data = setCheck();
    {
        // entry更新: 可能な限り前のentryの寿命を維持するようにする
        std::lock_guard lock(data->value_store.mtx);
        auto prev_entry_p =
            data->value_store.getEntryP(this->member_, this->field_);
        if (prev_entry_p) {
            if (prev_entry_p->shape != shape.fixed_shape) {
                prev_entry_p->shape = shape.fixed_shape;
            }
            prev_entry_p->fixed = shape.is_fixed;
        } else {
            data->value_store.setEntry(*this, shape);
        }
    }
    return setImpl(std::move(v));
}
const Value &Value::setImpl(std::vector<double> v) const {
    auto data = setCheck();
    data->value_store.setSend(
        *this, std::make_shared<std::vector<double>>(std::move(v)));
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
        parent.setImpl(*pv, shape);
        return *this;
    } else {
        throw std::out_of_range("index out of range, current size: " +
                                std::to_string(pv ? pv->size() : 0) +
                                ", got index: " + std::to_string(index));
    }
}
const Value &Value::set(double v) const {
    setImpl(std::vector<double>{v}, ValueShape{{}, true});
    return *this;
}
const Value &Value::set(std::vector<double> v) const {
    return setImpl(std::move(v), ValueShape{{}, false});
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
    this->setImpl(*pv);
    return *this;
}
const Value &Value::push_back(double v) const {
    auto pv = this->tryGetVec();
    if (pv) {
        pv->push_back(v);
    } else {
        pv.emplace({v});
    }
    this->setImpl(*pv);
    return *this;
}

std::optional<double> ValueElement<>::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    Value(*this).request();
    if (v && index < v->size()) {
        return v->at(index);
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
const std::vector<double> &Value::getVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return *v;
    } else {
        static std::vector<double> empty;
        return empty;
    }
}

std::optional<std::vector<double>> Value::tryGetVec(std::ptrdiff_t index,
                                                    std::ptrdiff_t size) const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        assert(index >= 0 && size >= 0 &&
               static_cast<std::size_t>(index + size) <= v->size());
        return std::vector<double>(v->begin() + index,
                                   v->begin() + index + size);
    } else {
        return std::nullopt;
    }
}
bool Value::tryGetArray(double *target, std::ptrdiff_t index,
                        std::ptrdiff_t size) const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        assert(index >= 0 && size >= 0 &&
               static_cast<std::size_t>(index + size) <= v->size());
        std::copy(v->begin() + index, v->begin() + index + size, target);
        return true;
    } else {
        return false;
    }
}

std::size_t Value::size() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    return v ? v->size() : 0;
}
bool Value::isFixed() const {
    auto data = dataLock();
    std::lock_guard lock(data->value_store.mtx);
    auto e = data->value_store.getEntryP(this->member_, this->field_);
    return e && e->fixed;
}
const std::vector<std::size_t> &Value::fixedShape() const {
    auto data = dataLock();
    std::lock_guard lock(data->value_store.mtx);
    auto e = data->value_store.getEntryP(this->member_, this->field_);
    if (e) {
        auto &shape = e->shape;
        if (shape.empty()) {
            shape.push_back(1);
        }
        return shape;
    } else {
        static std::vector<std::size_t> empty;
        return empty;
    }
}
std::size_t Value::fixedSize() const {
    auto data = dataLock();
    std::lock_guard lock(data->value_store.mtx);
    auto e = data->value_store.getEntryP(this->member_, this->field_);
    if (e) {
        auto &shape = e->shape;
        return std::accumulate(shape.begin(), shape.end(),
                               static_cast<std::size_t>(1),
                               [](auto a, auto b) { return a * b; });
    } else {
        return 0;
    }
}

void Value::assertSize(std::size_t size, bool fixed) const {
    if (dataLock()->isSelf(*this)) {
        if (fixed) {
            resize(size);
        }
    } else {
        auto v = dataLock()->value_store.getRecv(*this);
        if (fixed) {
            if (v && v->size() != size) {
                throw std::runtime_error(
                    "array size mismatch, expected: " + std::to_string(size) +
                    ", actual data size is: " + std::to_string(v->size()));
            }
        } else {
            if (v && v->size() % size != 0) {
                throw std::runtime_error(
                    "array size mismatch, expected multiple of: " +
                    std::to_string(size) +
                    ", actual data size is: " + std::to_string(v->size()));
            }
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
    std::lock_guard lock(dataLock()->value_store.mtx);
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
