#include "webcface/robot_model.h"
#include "webcface/member.h"
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"
WEBCFACE_NS_BEGIN

RobotModel::RobotModel()
    : Field(), Canvas3DComponent(Canvas3DComponentType::robot_model),
      sb(std::make_shared<internal::DataSetBuffer<RobotLink>>()) {}

RobotModel::RobotModel(const Field &base)
    : Field(base),
      Canvas3DComponent(Canvas3DComponentType::robot_model, this->dataLock()),
      sb(std::make_shared<internal::DataSetBuffer<RobotLink>>(base)) {
    this->Canvas3DComponent::robotModel(*this);
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
void internal::DataSetBuffer<RobotLink>::onSync() {
    auto ls = std::make_shared<std::vector<RobotLink>>(std::move(components_));
    auto data = target_.setCheck();
    data->robot_model_store.setSend(target_, ls);
    std::shared_ptr<std::function<void(RobotModel)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event =
            data->robot_model_change_event[target_.member_][target_.field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
RobotModel &RobotModel::onChange(std::function<void(RobotModel)> callback) {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->robot_model_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(RobotModel)>>(std::move(callback));
    return *this;
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
        data->messagePushOnline(message::packSingle(
            message::Req<message::RobotModel>{{}, member_, field_, req}));
    }
}

RobotModel &RobotModel::set(const std::vector<RobotLink> &v) {
    sb->set(v); // set()のなかでchange_eventは呼ばれる
    return *this;
}

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
