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

std::string Member::name() const {
    if (!memberPtr()) {
        throw std::invalid_argument("member name is null");
    }
    return Encoding::getName(memberPtr());
}
std::wstring Member::nameW() const {
    if (!memberPtr()) {
        throw std::invalid_argument("member name is null");
    }
    return Encoding::getNameW(memberPtr());
}

Value Member::value(const std::string &field) const {
    return Value{*this, field};
}
Text Member::text(const std::string &field) const { return Text{*this, field}; }
RobotModel Member::robotModel(const std::string &field) const {
    return RobotModel{*this, field};
}
Image Member::image(const std::string &field) const {
    return Image{*this, field};
}
Func Member::func(const std::string &field) const { return Func{*this, field}; }
View Member::view(const std::string &field) const { return View{*this, field}; }
Canvas3D Member::canvas3D(const std::string &field) const {
    return Canvas3D{*this, field};
}
Canvas2D Member::canvas2D(const std::string &field) const {
    return Canvas2D{*this, field};
}
Value Member::value(const std::wstring &field) const {
    return Value{*this, field};
}
Text Member::text(const std::wstring &field) const {
    return Text{*this, field};
}
RobotModel Member::robotModel(const std::wstring &field) const {
    return RobotModel{*this, field};
}
Image Member::image(const std::wstring &field) const {
    return Image{*this, field};
}
Func Member::func(const std::wstring &field) const {
    return Func{*this, field};
}
View Member::view(const std::wstring &field) const {
    return View{*this, field};
}
Canvas3D Member::canvas3D(const std::wstring &field) const {
    return Canvas3D{*this, field};
}
Canvas2D Member::canvas2D(const std::wstring &field) const {
    return Canvas2D{*this, field};
}
Log Member::log() const { return Log{*this}; }

EventTarget<Value, MemberNamePtr> Member::onValueEntry() const {
    return EventTarget<Value, MemberNamePtr>{&dataLock()->value_entry_event,
                                             member_};
}
EventTarget<Text, MemberNamePtr> Member::onTextEntry() const {
    return EventTarget<Text, MemberNamePtr>{&dataLock()->text_entry_event,
                                            member_};
}
EventTarget<RobotModel, MemberNamePtr> Member::onRobotModelEntry() const {
    return EventTarget<RobotModel, MemberNamePtr>{
        &dataLock()->robot_model_entry_event, member_};
}
EventTarget<Func, MemberNamePtr> Member::onFuncEntry() const {
    return EventTarget<Func, MemberNamePtr>{&dataLock()->func_entry_event,
                                            member_};
}
EventTarget<View, MemberNamePtr> Member::onViewEntry() const {
    return EventTarget<View, MemberNamePtr>{&dataLock()->view_entry_event,
                                            member_};
}
EventTarget<Canvas3D, MemberNamePtr> Member::onCanvas3DEntry() const {
    return EventTarget<Canvas3D, MemberNamePtr>{
        &dataLock()->canvas3d_entry_event, member_};
}
EventTarget<Canvas2D, MemberNamePtr> Member::onCanvas2DEntry() const {
    return EventTarget<Canvas2D, MemberNamePtr>{
        &dataLock()->canvas2d_entry_event, member_};
}
EventTarget<Image, MemberNamePtr> Member::onImageEntry() const {
    return EventTarget<Image, MemberNamePtr>{&dataLock()->image_entry_event,
                                             member_};
}
EventTarget<Member, MemberNamePtr> Member::onSync() const {
    return EventTarget<Member, MemberNamePtr>{&dataLock()->sync_event, member_};
}

std::vector<Value> Member::valueEntries() const {
    auto keys = dataLock()->value_store.getEntry(*this);
    std::vector<Value> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = value(keys[i]);
    }
    return ret;
}
std::vector<Text> Member::textEntries() const {
    auto keys = dataLock()->text_store.getEntry(*this);
    std::vector<Text> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = text(keys[i]);
    }
    return ret;
}
std::vector<RobotModel> Member::robotModelEntries() const {
    auto keys = dataLock()->robot_model_store.getEntry(*this);
    std::vector<RobotModel> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = robotModel(keys[i]);
    }
    return ret;
}
std::vector<Func> Member::funcEntries() const {
    auto keys = dataLock()->func_store.getEntry(*this);
    std::vector<Func> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = func(keys[i]);
    }
    return ret;
}
std::vector<View> Member::viewEntries() const {
    auto keys = dataLock()->view_store.getEntry(*this);
    std::vector<View> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = view(keys[i]);
    }
    return ret;
}
std::vector<Canvas3D> Member::canvas3DEntries() const {
    auto keys = dataLock()->canvas3d_store.getEntry(*this);
    std::vector<Canvas3D> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = canvas3D(keys[i]);
    }
    return ret;
}
std::vector<Canvas2D> Member::canvas2DEntries() const {
    auto keys = dataLock()->canvas2d_store.getEntry(*this);
    std::vector<Canvas2D> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = canvas2D(keys[i]);
    }
    return ret;
}
std::vector<Image> Member::imageEntries() const {
    auto keys = dataLock()->image_store.getEntry(*this);
    std::vector<Image> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = image(keys[i]);
    }
    return ret;
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
EventTarget<Member, MemberNamePtr> Member::onPing() const {
    // ほんとはonAppendに追加したかったけど面倒なのでここでリクエストをtrueにしちゃう
    dataLock()->pingStatusReq();
    return EventTarget<Member, MemberNamePtr>{&dataLock()->ping_event, member_};
}
WEBCFACE_NS_END
