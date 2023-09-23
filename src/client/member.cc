#include <webcface/member.h>
#include <webcface/data.h>
#include <webcface/func.h>
#include <webcface/view.h>
#include <webcface/event_target.h>
#include <webcface/client_data.h>
#include "../message/message.h"
namespace WebCFace {

Value Member::value(const std::string &field) const {
    return Value{*this, field};
}
Text Member::text(const std::string &field) const { return Text{*this, field}; }
Func Member::func(const std::string &field) const { return Func{*this, field}; }
View Member::view(const std::string &field) const { return View{*this, field}; }
Log Member::log() const { return Log{*this}; }

EventTarget<Value, std::string> Member::onValueEntry() const {
    return EventTarget<Value, std::string>{&dataLock()->value_entry_event,
                                           member_};
}
EventTarget<Text, std::string> Member::onTextEntry() const {
    return EventTarget<Text, std::string>{&dataLock()->text_entry_event,
                                          member_};
}
EventTarget<Func, std::string> Member::onFuncEntry() const {
    return EventTarget<Func, std::string>{&dataLock()->func_entry_event,
                                          member_};
}
EventTarget<View, std::string> Member::onViewEntry() const {
    return EventTarget<View, std::string>{&dataLock()->view_entry_event,
                                          member_};
}
EventTarget<Member, std::string> Member::onSync() const {
    return EventTarget<Member, std::string>{&dataLock()->sync_event, member_};
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

std::string Member::libName() const {
    auto data = dataLock();
    return data->member_lib_name.at(data->getMemberIdFromName(member_));
}
std::string Member::libVersion() const {
    auto data = dataLock();
    return data->member_lib_ver.at(data->getMemberIdFromName(member_));
}
std::string Member::remoteAddr() const {
    auto data = dataLock();
    return data->member_addr.at(data->getMemberIdFromName(member_));
}

std::optional<int> Member::pingStatus() const {
    auto data = dataLock();
    if (!data->ping_status_req) {
        data->message_queue.push(Message::packSingle(Message::PingStatusReq{}));
        data->ping_status_req = true;
    }
    if (data->ping_status.count(data->getMemberIdFromName(member_))) {
        return data->ping_status.at(data->getMemberIdFromName(member_))
    } else {
        return std::nullopt;
    }
}
EventTarget<Member, std::string> Member::onPing() const {
    // ほんとはonAppendに追加したかったけど面倒なのでここでpingStatus呼び出してリクエストをtrueにしちゃう
    pingStatus();
    return EventTarget<Member, std::string>{&data->ping_event, member_};
}
} // namespace WebCFace