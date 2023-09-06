#include <webcface/member.h>
#include <webcface/data.h>
#include <webcface/func.h>
#include <webcface/view.h>
#include <webcface/event_target.h>
#include <webcface/client_data.h>

namespace WebCFace {

Value Member::value(const std::string &field) const {
    return Value{*this, field};
}
Text Member::text(const std::string &field) const { return Text{*this, field}; }
Func Member::func(const std::string &field) const { return Func{*this, field}; }
View Member::view(const std::string &field) const { return View{*this, field}; }
Log Member::log() const { return Log{*this}; }

EventTarget<Value, eventpp::EventDispatcher<std::string, void(Field)>>
Member::onValueEntry() const {
    return EventTarget<Value,
                       eventpp::EventDispatcher<std::string, void(Field)>>{
        &dataLock()->value_entry_event, member_};
}
EventTarget<Text, eventpp::EventDispatcher<std::string, void(Field)>>
Member::onTextEntry() const {
    return EventTarget<Text,
                       eventpp::EventDispatcher<std::string, void(Field)>>{
        &dataLock()->text_entry_event, member_};
}
EventTarget<Func, eventpp::EventDispatcher<std::string, void(Field)>>
Member::onFuncEntry() const {
    return EventTarget<Func,
                       eventpp::EventDispatcher<std::string, void(Field)>>{
        &dataLock()->func_entry_event, member_};
}
EventTarget<View, eventpp::EventDispatcher<std::string, void(Field)>>
Member::onViewEntry() const {
    return EventTarget<View,
                       eventpp::EventDispatcher<std::string, void(Field)>>{
        &dataLock()->view_entry_event, member_};
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
std::vector<View> Member::views() const {
    auto keys = dataLock()->view_store.getEntry(*this);
    std::vector<View> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = view(keys[i]);
    }
    return ret;
}

} // namespace WebCFace