#include "webcface/internal/client_internal.h"
#include "webcface/value.h"
#include "webcface/member.h"
#include "webcface/message/message.h"
#include <algorithm>
#include <cctype>

WEBCFACE_NS_BEGIN

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

const Value &Value::set(double v) const {
    auto last_name = this->lastName();
    auto parent = this->parent();
    if (std::all_of(last_name.cbegin(), last_name.cend(),
                    [](unsigned char c) { return std::isdigit(c); })) {
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
    set(std::vector<double>{v});
    return *this;
}

const Value &Value::set(std::vector<double> v) const {
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

std::optional<double> Value::tryGet() const {
    auto v = dataLock()->value_store.getRecv(*this);
    request();
    if (v) {
        return (*v)->size() >= 1 ? std::make_optional((**v)[0]) : std::nullopt;
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
