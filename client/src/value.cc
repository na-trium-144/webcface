#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/internal/client_internal.h"
#include "webcface/value.h"
#include "webcface/member.h"
#include <algorithm>
#include <cctype>

WEBCFACE_NS_BEGIN

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

const Value &Value::set(double v) const {
    auto last_name = this->lastName();
    auto parent = this->parent();
    auto data = setCheck();
    // deprecated in ver2.8
    if (std::all_of(last_name.cbegin(), last_name.cend(),
                    [](unsigned char c) { return std::isdigit(c); })) {
        std::size_t index = std::stoi(std::string(last_name));
        auto pv = data->value_store.getRecv(parent);
        if (pv && index < pv->size() + 10) { // てきとう
            while (pv->size() <= index) {
                pv->push_back(0);
            }
            pv->at(index) = v;
            data->value_store.setSend(parent, *pv);
            return *this;
        }
    }
    data->value_store.setSend(*this, MutableNumVector{v});
    auto change_event =
        internal::findFromMap2(data->value_change_event.shared_lock().get(),
                               this->member_, this->field_);
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}

const Value &Value::set(std::vector<double> v) const {
    auto data = setCheck();
    data->value_store.setSend(*this, MutableNumVector{std::move(v)});
    auto change_event =
        internal::findFromMap2(data->value_change_event.shared_lock().get(),
                               this->member_, this->field_);
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}
const Value &Value::onChange(std::function<void(Value)> callback) const {
    this->request();
    auto data = dataLock();
    data->value_change_event.lock().get()[this->member_][this->field_] =
        std::make_shared<std::function<void(Value)>>(std::move(callback));
    return *this;
}

const Value &Value::resize(std::size_t size) const {
    auto data = setCheck();
    auto pv = data->value_store.getRecv(*this);
    if (pv) {
        pv->resize(size);
    } else {
        pv.emplace(std::vector<double>(size));
    }
    data->value_store.setSend(*this, *pv);
    return *this;
}
const Value &Value::push_back(double v) const {
    auto data = setCheck();
    auto pv = data->value_store.getRecv(*this);
    if (pv) {
        pv->push_back(v);
    } else {
        pv.emplace(v);
    }
    data->value_store.setSend(*this, *pv);
    return *this;
}
std::size_t Value::size() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return v->size();
    } else {
        return 0;
    }
}

const ValueElementRef &ValueElementRef::set(double v) const {
    Value parent = *this;
    auto data = setCheck();
    auto pv = data->value_store.getRecv(parent);
    if (pv && index < pv->size()) {
        pv->at(index) = v;
        data->value_store.setRecv(parent, *pv);
    } else {
        throw OutOfRange("ValueElementRef::set got index " +
                         std::to_string(index) + " but size is " +
                         std::to_string(pv->size()));
    }
    return *this;
}

std::optional<double> Value::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return std::make_optional((*v)[0]);
    }
    // deprecated in ver2.8
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
std::optional<NumVector> Value::tryGetVec() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return *v;
    } else {
        return std::nullopt;
    }
}
std::optional<double> ValueElementRef::tryGet() const {
    Value parent = *this;
    auto pv = parent.tryGetVec();
    if (pv && index < pv->size()) {
        return pv->at(index);
    } else {
        return std::nullopt;
    }
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
