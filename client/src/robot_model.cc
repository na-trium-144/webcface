#include <webcface/robot_model.h>
#include <webcface/member.h>
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/event_target_impl.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF EventTarget<RobotModel>;

RobotModel::RobotModel()
    : Field(), EventTarget<RobotModel>(),
      Canvas3DComponent(Canvas3DComponentType::robot_model),
      sb(std::make_shared<Internal::DataSetBuffer<RobotLink>>()) {}

RobotModel::RobotModel(const Field &base)
    : Field(base), EventTarget<RobotModel>(),
      Canvas3DComponent(Canvas3DComponentType::robot_model, this->dataLock()),
      sb(std::make_shared<Internal::DataSetBuffer<RobotLink>>(base)) {
    this->Canvas3DComponent::robotModel(*this);
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(this->dataLock()->robot_model_change_event[this->member_][this->field_]);
}

RobotModel &RobotModel::init() {
    sb->init();
    return *this;
}
RobotModel &RobotModel::sync() {
    sb->sync();
    return *this;
}
template <>
void Internal::DataSetBuffer<RobotLink>::onSync() {
    auto ls = std::make_shared<std::vector<RobotLink>>(std::move(components_));
    target_.setCheck()->robot_model_store.setSend(target_, ls);
    static_cast<RobotModel>(target_).triggerEvent(target_);
}
RobotModel &RobotModel::operator<<(const RobotLink &vc) {
    sb->add(vc);
    return *this;
}
RobotModel &RobotModel::operator<<(RobotLink &&vc) {
    sb->add(std::move(vc));
    return *this;
}

void RobotModel::request() const {
    auto data = dataLock();
    auto req = data->robot_model_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::RobotModel>{{}, member_, field_, req}));
    }
}

RobotModel &RobotModel::set(const std::vector<RobotLink> &v) {
    sb->set(v);
    return *this;
}

void RobotModel::onAppend() const { request(); }

std::optional<std::vector<RobotLink>> RobotModel::tryGet() const {
    request();
    auto v = dataLock()->robot_model_store.getRecv(*this);
    if (v) {
        return **v;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point RobotModel::time() const {
    return member().syncTime();
}
RobotModel &RobotModel::free() {
    auto req = dataLock()->robot_model_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

WEBCFACE_NS_END
