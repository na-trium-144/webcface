#include <webcface/member.h>
#include <webcface/data.h>
#include <webcface/func.h>
#include <webcface/event_target.h>
#include <webcface/client_data.h>

namespace WebCFace {

Value Member::value(const std::string &field) const {
    return Value{*this, field};
}
Text Member::text(const std::string &field) const { return Text{*this, field}; }
Func Member::func(const std::string &field) const { return Func{*this, field}; }
Logs Member::logs() const { return Logs{*this}; }

EventTarget<Value> Member::valuesChange() const {
    return EventTarget<Value>{EventType::value_entry, *this};
}
EventTarget<Text> Member::textsChange() const {
    return EventTarget<Text>{EventType::text_entry, *this};
}
EventTarget<Func> Member::funcsChange() const {
    return EventTarget<Func>{EventType::func_entry, *this};
}

std::vector<Value> Member::values() const {
    auto keys = dataLock()->value_store.getEntry(*this);
    std::vector<Value> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = value(keys[i]);
    }
    return ret;
}
std::vector<Text> Member::texts() const {
    auto keys = dataLock()->text_store.getEntry(*this);
    std::vector<Text> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = text(keys[i]);
    }
    return ret;
}
std::vector<Func> Member::funcs() const {
    auto keys = dataLock()->func_store.getEntry(*this);
    std::vector<Func> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = func(keys[i]);
    }
    return ret;
}

} // namespace WebCFace