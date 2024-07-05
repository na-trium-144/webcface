#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/robot_model.h>
#include <webcface/image.h>
#include <webcface/view.h>
#include <webcface/log.h>
#include <webcface/canvas2d.h>
#include <webcface/canvas3d.h>
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN

Log Member::log() const { return Log{*this}; }

Member &Member::onValueEntry(std::function<void(Value)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->value_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onTextEntry(std::function<void(Text)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->text_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onRobotModelEntry(std::function<void(RobotModel)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->robot_model_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onFuncEntry(std::function<void(Func)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->func_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onViewEntry(std::function<void(View)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->view_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onCanvas3DEntry(std::function<void(Canvas3D)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->canvas3d_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onCanvas2DEntry(std::function<void(Canvas2D)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->canvas2d_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onImageEntry(std::function<void(Image)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->image_entry_event[member_] = std::move(callback);
    return *this;
}
Member &Member::onSync(std::function<void(Member)> callback) {
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->sync_event[member_] = std::move(callback);
    return *this;
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
Member &Member::onPing(std::function<void(Member)> callback) {
    dataLock()->pingStatusReq();
    std::lock_guard lock(dataLock()->event_m);
    dataLock()->ping_event[member_] = std::move(callback);
    return *this;
}
WEBCFACE_NS_END
