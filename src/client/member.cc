#include <webcface/member.h>
#include <webcface/event_target.h>
#include "../message/message.h"
#include "client_internal.h"

namespace WEBCFACE_NS {

EventTarget<Value, std::string> Member::onValueEntry() const {
    return EventTarget<Value, std::string>{&dataLock()->value_entry_event,
                                           member_};
}
EventTarget<Text, std::string> Member::onTextEntry() const {
    return EventTarget<Text, std::string>{&dataLock()->text_entry_event,
                                          member_};
}
EventTarget<RobotModel, std::string> Member::onRobotModelEntry() const {
    return EventTarget<RobotModel, std::string>{
        &dataLock()->robot_model_entry_event, member_};
}
EventTarget<Func, std::string> Member::onFuncEntry() const {
    return EventTarget<Func, std::string>{&dataLock()->func_entry_event,
                                          member_};
}
EventTarget<View, std::string> Member::onViewEntry() const {
    return EventTarget<View, std::string>{&dataLock()->view_entry_event,
                                          member_};
}
EventTarget<Canvas3D, std::string> Member::onCanvas3DEntry() const {
    return EventTarget<Canvas3D, std::string>{&dataLock()->canvas3d_entry_event,
                                              member_};
}
EventTarget<Canvas2D, std::string> Member::onCanvas2DEntry() const {
    return EventTarget<Canvas2D, std::string>{&dataLock()->canvas2d_entry_event,
                                              member_};
}
EventTarget<Image, std::string> Member::onImageEntry() const {
    return EventTarget<Image, std::string>{&dataLock()->image_entry_event,
                                           member_};
}
EventTarget<Member, std::string> Member::onSync() const {
    return EventTarget<Member, std::string>{&dataLock()->sync_event, member_};
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
EventTarget<Member, std::string> Member::onPing() const {
    // ほんとはonAppendに追加したかったけど面倒なのでここでリクエストをtrueにしちゃう
    dataLock()->pingStatusReq();
    return EventTarget<Member, std::string>{&dataLock()->ping_event, member_};
}
} // namespace WEBCFACE_NS
