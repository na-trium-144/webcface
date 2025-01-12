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

void Value::assertSize(const ValueShape &shape, std::size_t write_size,
                       bool overwrite_entry) const {
    auto size = std::accumulate(
        shape.fixed_shape.begin(), shape.fixed_shape.end(),
        static_cast<std::size_t>(1), [](auto a, auto b) { return a * b; });
    assert(!(overwrite_entry && write_size == 0));
    auto data = dataLock();
    std::lock_guard lock(data->value_store.mtx);
    auto e = data->value_store.getEntryP(this->member_, this->field_);
    auto v = data->value_store.getRecv(*this);
    assert(!(v && !e));
    if (e) {
        auto e_size = std::accumulate(e->shape.begin(), e->shape.end(),
                                      static_cast<std::size_t>(1),
                                      [](auto a, auto b) { return a * b; });
        bool match = (e_size == size && e->fixed) ||
                     (e_size % size == 0 && !shape.is_fixed) ||
                     (v && v->size() % size == 0 && !shape.is_fixed) ||
                     (e_size == 1 && !e->fixed && size == 1) ||
                     (e_size == 1 && !e->fixed && !v) ||
                     (e_size == 1 && !e->fixed && v && v->size() == size);
        if (!match) {
            throw std::invalid_argument(
                "array size mismatch, current data shape is " +
                std::to_string(e_size) + (e->fixed ? " (Fixed)" : "") +
                ", current data size is " + std::to_string(v ? v->size() : 0) +
                ", and tried to access it as shape " + std::to_string(size) +
                (shape.is_fixed ? " (Fixed)" : ""));
        }
        if (write_size > 0) {
            bool write_match = (write_size == e_size) ||
                               (write_size % e_size == 0 && !e->fixed) ||
                               (e_size == 1);
            if (!write_match) {
                throw std::invalid_argument(
                    "array size mismatch, expected " + std::to_string(e_size) +
                    (e->fixed ? " (Fixed)" : "") + " but got " +
                    std::to_string(write_size));
            }
        }
    }
    if (overwrite_entry) {
        setCheck();
        // entry更新: 可能な限り前のentryの寿命を維持するようにする
        if (e) {
            if (e->shape != shape.fixed_shape) {
                e->shape = shape.fixed_shape;
            }
            e->fixed = shape.is_fixed;
        } else {
            data->value_store.setEntry(*this, shape);
        }
    }
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
        parent.assertSize(original_shape, pv->size(), true);
        parent.setImpl(*pv);
        return *this;
    } else {
        throw std::out_of_range("index out of range, current size: " +
                                std::to_string(pv ? pv->size() : 0) +
                                ", got index: " + std::to_string(index));
    }
}
const Value &Value::set(double v) const {
    assertSize({{1}, true}, 1, true);
    setImpl(std::vector<double>{v});
    return *this;
}
const Value &Value::set(std::vector<double> v) const {
    assertSize({{1}, false}, v.size(), true);
    return setImpl(std::move(v));
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
    this->resizeImpl(size, true);
    return *this;
}
const Value &Value::resizeImpl(std::size_t size, bool overwrite_entry) const {
    auto pv = this->tryGetVec();
    if (pv) {
        pv->resize(size);
    } else {
        pv.emplace(size);
    }
    this->assertSize({{1}, false}, size, overwrite_entry);
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
    this->assertSize({{1}, false}, pv->size(), true);
    this->setImpl(*pv);
    return *this;
}

std::optional<double> ValueElement<>::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    Value parent = *this;
    parent.assertSize(original_shape, 0, false);
    parent.request();
    if (v && index < v->size()) {
        return v->at(index);
    }
    return std::nullopt;
}
std::optional<double> Value::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    assertSize({{1}, true}, 0, false);
    request();
    if (v && v->size() >= 1) {
        return (*v)[0];
    }
    return std::nullopt;
}
std::optional<std::vector<double>> Value::tryGetVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    // assertSize({{1}, false}, false);
    request();
    if (v) {
        return *v;
    } else {
        return std::nullopt;
    }
}
const std::vector<double> &Value::getVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    assertSize({{1}, false}, 0, false);
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
    // assertSize({1}, false, false);
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
    // assertSize({1}, false, false);
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
