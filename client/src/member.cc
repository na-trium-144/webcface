#include "webcface/member.h"
#include "webcface/value.h"
#include "webcface/text.h"
#include "webcface/robot_model.h"
#include "webcface/image.h"
#include "webcface/view.h"
#include "webcface/log.h"
#include "webcface/canvas2d.h"
#include "webcface/canvas3d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/common/internal/message/log.h"

WEBCFACE_NS_BEGIN

// ver2.4〜: nameを省略した場合 "default" として送信される。
template <typename T, bool>
T Member::log() const {
    return Log{*this, message::Log::defaultLogName()};
}
template WEBCFACE_DLL Log Member::log<Log, true>() const;

bool Member::connected() const {
    auto data = this->dataLock();
    if (data->isSelf(*this)) {
        return data->ws_data.lock()->connected;
    } else {
        auto lock_entry = data->member_entry.shared_lock();
        return lock_entry->count(this->member_) &&
               lock_entry->at(this->member_);
    }
}
const Member &Member::onDisconnect(
    std::function<void WEBCFACE_CALL_FP(Member)> callback) const {
    dataLock()->member_disconnected_event.lock().get()[member_] =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
    return *this;
}
const Member &Member::onConnect(
    std::function<void WEBCFACE_CALL_FP(Member)> callback) const {
    dataLock()->member_connected_event.lock().get()[member_] =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
    return *this;
}

const Member &Member::onValueEntry(std::function<void(Value)> callback) const {
    dataLock()->value_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Value)>>(std::move(callback));
    return *this;
}
const Member &Member::onTextEntry(std::function<void(Text)> callback) const {
    dataLock()->text_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Text)>>(std::move(callback));
    return *this;
}
const Member &
Member::onRobotModelEntry(std::function<void(RobotModel)> callback) const {
    dataLock()->robot_model_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(RobotModel)>>(std::move(callback));
    return *this;
}
const Member &Member::onFuncEntry(std::function<void(Func)> callback) const {
    dataLock()->func_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Func)>>(std::move(callback));
    return *this;
}
const Member &Member::onViewEntry(std::function<void(View)> callback) const {
    dataLock()->view_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(View)>>(std::move(callback));
    return *this;
}
const Member &
Member::onCanvas3DEntry(std::function<void(Canvas3D)> callback) const {
    dataLock()->canvas3d_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Canvas3D)>>(std::move(callback));
    return *this;
}
const Member &
Member::onCanvas2DEntry(std::function<void(Canvas2D)> callback) const {
    dataLock()->canvas2d_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Canvas2D)>>(std::move(callback));
    return *this;
}
const Member &Member::onImageEntry(std::function<void(Image)> callback) const {
    dataLock()->image_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Image)>>(std::move(callback));
    return *this;
}
const Member &Member::onLogEntry(std::function<void(Log)> callback) const {
    dataLock()->log_entry_event.lock().get()[member_] =
        std::make_shared<std::function<void(Log)>>(std::move(callback));
    return *this;
}
const Member &Member::onSync(std::function<void(Member)> callback) const {
    dataLock()->sync_event.lock().get()[member_] =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
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

const std::string &Member::libName() const {
    auto data = dataLock();
    auto m_id = data->getMemberIdFromName(member_);
    auto lock_name = data->member_lib_name.shared_lock();
    if (lock_name->count(m_id)) {
        return lock_name->at(m_id);
    } else {
        return SharedString::emptyStr();
    }
}
const std::string &Member::libVersion() const {
    auto data = dataLock();
    auto m_id = data->getMemberIdFromName(member_);
    auto lock_ver = data->member_lib_ver.shared_lock();
    if (lock_ver->count(m_id)) {
        return lock_ver->at(m_id);
    } else {
        return SharedString::emptyStr();
    }
}
const std::string &Member::remoteAddr() const {
    auto data = dataLock();
    auto m_id = data->getMemberIdFromName(member_);
    auto lock_addr = data->member_addr.shared_lock();
    if (lock_addr->count(m_id)) {
        return lock_addr->at(m_id);
    } else {
        return SharedString::emptyStr();
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
const Member &Member::onPing(std::function<void(Member)> callback) const {
    dataLock()->pingStatusReq();
    dataLock()->ping_event.lock().get()[member_] =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
    return *this;
}
WEBCFACE_NS_END
