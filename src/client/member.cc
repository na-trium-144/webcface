#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/robot_model.h>
#include <webcface/image.h>
#include <webcface/view.h>
#include <webcface/log.h>
#include <webcface/canvas2d.h>
#include <webcface/canvas3d.h>
#include <webcface/event_target.h>
#include <webcface/encoding.h>
#include "../message/message.h"
#include "client_internal.h"

WEBCFACE_NS_BEGIN

Log Member::log() const { return Log{*this}; }

EventTarget<Value> Member::onValueEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Value>{&dataLock()->value_entry_event[member_]};
}
EventTarget<Text> Member::onTextEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Text>{&dataLock()->text_entry_event[member_]};
}
EventTarget<RobotModel> Member::onRobotModelEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<RobotModel>{
        &dataLock()->robot_model_entry_event[member_]};
}
EventTarget<Func> Member::onFuncEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Func>{&dataLock()->func_entry_event[member_]};
}
EventTarget<View> Member::onViewEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<View>{&dataLock()->view_entry_event[member_]};
}
EventTarget<Canvas3D> Member::onCanvas3DEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Canvas3D>{&dataLock()->canvas3d_entry_event[member_]};
}
EventTarget<Canvas2D> Member::onCanvas2DEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Canvas2D>{&dataLock()->canvas2d_entry_event[member_]};
}
EventTarget<Image> Member::onImageEntry() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Image>{&dataLock()->image_entry_event[member_]};
}
EventTarget<Member> Member::onSync() const {
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Member>{&dataLock()->sync_event[member_]};
}

std::vector<Value> Member::values() const { return valueEntries(); }
std::vector<Text> Member::texts() const { return textEntries(); }
std::vector<Func> Member::funcs() const { return funcEntries(); }
std::vector<View> Member::views() const { return viewEntries(); }
std::vector<Image> Member::images() const { return imageEntries(); }
std::vector<RobotModel> Member::robotModels() const {
    return robotModelEntries();
}


std::chrono::system_clock::time_point Member::syncTime() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}

std::string Member::libName() const {
    auto data = dataLock();
    auto m_id = data->getMemberIdFromName(member_);
    if (data->member_lib_name.count(m_id)) {
        return data->member_lib_name.at(m_id);
    } else {
        return "";
    }
}
std::string Member::libVersion() const {
    auto data = dataLock();
    auto m_id = data->getMemberIdFromName(member_);
    if (data->member_lib_ver.count(m_id)) {
        return data->member_lib_ver.at(m_id);
    } else {
        return "";
    }
}
std::string Member::remoteAddr() const {
    auto data = dataLock();
    auto m_id = data->getMemberIdFromName(member_);
    if (data->member_addr.count(m_id)) {
        return data->member_addr.at(m_id);
    } else {
        return "";
    }
}

std::optional<int> Member::pingStatus() const {
    auto data = dataLock();
    data->pingStatusReq();
    if (data->ping_status != nullptr &&
        data->ping_status->count(data->getMemberIdFromName(member_))) {
        return data->ping_status->at(data->getMemberIdFromName(member_));
    } else {
        return std::nullopt;
    }
}
EventTarget<Member> Member::onPing() const {
    // ほんとはonAppendに追加したかったけど面倒なのでここでリクエストをtrueにしちゃう
    dataLock()->pingStatusReq();
    std::lock_guard lock(dataLock()->event_m);
    return EventTarget<Member>{&dataLock()->ping_event[member_]};
}
WEBCFACE_NS_END
