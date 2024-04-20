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
    if (!member_) {
        throw std::invalid_argument("member name is null");
    }
    return Encoding::getName(member_);
}
std::wstring Member::nameW() const {
    if (!member_) {
        throw std::invalid_argument("member name is null");
    }
    return Encoding::getNameW(member_);
}

Value Member::value(std::string_view field) const {
    return Value{*this, Encoding::initName(field)};
}
Value Member::value(std::wstring_view field) const {
    return Value{*this, Encoding::initNameW(field)};
}
Text Member::text(std::string_view field) const {
    return Text{*this, Encoding::initName(field)};
}
Text Member::text(std::wstring_view field) const {
    return Text{*this, Encoding::initNameW(field)};
}
RobotModel Member::robotModel(std::string_view field) const {
    return RobotModel{*this, Encoding::initName(field)};
}
RobotModel Member::robotModel(std::wstring_view field) const {
    return RobotModel{*this, Encoding::initNameW(field)};
}
Image Member::image(std::string_view field) const {
    return Image{*this, Encoding::initName(field)};
}
Image Member::image(std::wstring_view field) const {
    return Image{*this, Encoding::initNameW(field)};
}
Func Member::func(std::string_view field) const {
    return Func{*this, Encoding::initName(field)};
}
Func Member::func(std::wstring_view field) const {
    return Func{*this, Encoding::initNameW(field)};
}
View Member::view(std::string_view field) const {
    return View{*this, Encoding::initName(field)};
}
View Member::view(std::wstring_view field) const {
    return View{*this, Encoding::initNameW(field)};
}
Canvas3D Member::canvas3D(std::string_view field) const {
    return Canvas3D{*this, Encoding::initName(field)};
}
Canvas3D Member::canvas3D(std::wstring_view field) const {
    return Canvas3D{*this, Encoding::initNameW(field)};
}
Canvas2D Member::canvas2D(std::string_view field) const {
    return Canvas2D{*this, Encoding::initName(field)};
}
Canvas2D Member::canvas2D(std::wstring_view field) const {
    return Canvas2D{*this, Encoding::initNameW(field)};
}
Log Member::log() const { return Log{*this}; }

EventTarget<Value, MemberNameRef> Member::onValueEntry() const {
    return EventTarget<Value, MemberNameRef>{&dataLock()->value_entry_event,
                                             member_};
}
EventTarget<Text, MemberNameRef> Member::onTextEntry() const {
    return EventTarget<Text, MemberNameRef>{&dataLock()->text_entry_event,
                                            member_};
}
EventTarget<RobotModel, MemberNameRef> Member::onRobotModelEntry() const {
    return EventTarget<RobotModel, MemberNameRef>{
        &dataLock()->robot_model_entry_event, member_};
}
EventTarget<Func, MemberNameRef> Member::onFuncEntry() const {
    return EventTarget<Func, MemberNameRef>{&dataLock()->func_entry_event,
                                            member_};
}
EventTarget<View, MemberNameRef> Member::onViewEntry() const {
    return EventTarget<View, MemberNameRef>{&dataLock()->view_entry_event,
                                            member_};
}
EventTarget<Canvas3D, MemberNameRef> Member::onCanvas3DEntry() const {
    return EventTarget<Canvas3D, MemberNameRef>{
        &dataLock()->canvas3d_entry_event, member_};
}
EventTarget<Canvas2D, MemberNameRef> Member::onCanvas2DEntry() const {
    return EventTarget<Canvas2D, MemberNameRef>{
        &dataLock()->canvas2d_entry_event, member_};
}
EventTarget<Image, MemberNameRef> Member::onImageEntry() const {
    return EventTarget<Image, MemberNameRef>{&dataLock()->image_entry_event,
                                             member_};
}
EventTarget<Member, MemberNameRef> Member::onSync() const {
    return EventTarget<Member, MemberNameRef>{&dataLock()->sync_event, member_};
}

std::vector<Value> Member::valueEntries() const {
    auto keys = dataLock()->value_store.getEntry(*this);
    std::vector<Value> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = Value(*this, keys[i]);
    }
    return ret;
}
std::vector<Text> Member::textEntries() const {
    auto keys = dataLock()->text_store.getEntry(*this);
    std::vector<Text> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = Text(*this, keys[i]);
    }
    return ret;
}
std::vector<RobotModel> Member::robotModelEntries() const {
    auto keys = dataLock()->robot_model_store.getEntry(*this);
    std::vector<RobotModel> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = RobotModel(*this, keys[i]);
    }
    return ret;
}
std::vector<Func> Member::funcEntries() const {
    auto keys = dataLock()->func_store.getEntry(*this);
    std::vector<Func> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = Func(*this, keys[i]);
    }
    return ret;
}
std::vector<View> Member::viewEntries() const {
    auto keys = dataLock()->view_store.getEntry(*this);
    std::vector<View> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = View(*this, keys[i]);
    }
    return ret;
}
std::vector<Canvas3D> Member::canvas3DEntries() const {
    auto keys = dataLock()->canvas3d_store.getEntry(*this);
    std::vector<Canvas3D> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = Canvas3D(*this, keys[i]);
    }
    return ret;
}
std::vector<Canvas2D> Member::canvas2DEntries() const {
    auto keys = dataLock()->canvas2d_store.getEntry(*this);
    std::vector<Canvas2D> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = Canvas2D(*this, keys[i]);
    }
    return ret;
}
std::vector<Image> Member::imageEntries() const {
    auto keys = dataLock()->image_store.getEntry(*this);
    std::vector<Image> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = Image(*this, keys[i]);
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
EventTarget<Member, MemberNameRef> Member::onPing() const {
    // ほんとはonAppendに追加したかったけど面倒なのでここでリクエストをtrueにしちゃう
    dataLock()->pingStatusReq();
    return EventTarget<Member, MemberNameRef>{&dataLock()->ping_event, member_};
}
WEBCFACE_NS_END
