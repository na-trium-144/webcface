#include <webcface/webcface.h>
#include <string>

namespace WebCFace {

Value Member::value(const std::string &name) const {
    return Value{data, this->name(), name};
}
Text Member::text(const std::string &name) const {
    return Text{data, this->name(), name};
}
Func Member::func(const std::string &name) const {
    return Func{data, this->name(), name};
}

EventTarget<Value> Member::valuesChange() const {
    return EventTarget<Value>{data, EventType::value_entry, name_};
}
EventTarget<Text> Member::textsChange() const {
    return EventTarget<Text>{data, EventType::text_entry, name_};
}
EventTarget<Func> Member::funcsChange() const {
    return EventTarget<Func>{data, EventType::func_entry, name_};
}

std::vector<Value> Member::values() const {
    if (auto data_s = data.lock()) {
        auto keys = data_s->value_store.getEntry(this->name());
        std::vector<Value> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = value(keys[i]);
        }
        return ret;
    }
    return std::vector<Value>{};
}
std::vector<Text> Member::texts() const {
    if (auto data_s = data.lock()) {
        auto keys = data_s->text_store.getEntry(this->name());
        std::vector<Text> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = text(keys[i]);
        }
        return ret;
    }
    return std::vector<Text>{};
}
std::vector<Func> Member::funcs() const {
    if (auto data_s = data.lock()) {
        auto keys = data_s->func_store.getEntry(this->name());
        std::vector<Func> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = func(keys[i]);
        }
        return ret;
    }
    return std::vector<Func>{};
}

} // namespace WebCFace