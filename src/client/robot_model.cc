#include <webcface/robot_model.h>
#include <webcface/member.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {
RobotModel::RobotModel(const Field &base)
    : Field(base), EventTarget<RobotModel>(
                       &this->dataLock()->robot_model_change_event, *this) {}

void RobotModel::request() const {
    auto data = dataLock();
    auto req = data->robot_model_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::RobotModel>{{}, member_, field_, req}));
    }
}

RobotModel &RobotModel::set(const std::vector<RobotLink> &v) {
    setCheck()->robot_model_store.setSend(*this, v);
    this->triggerEvent(*this);
    return *this;
}

void RobotModel::onAppend() const { request(); }

std::optional<std::vector<RobotLink>> RobotModel::tryGet() const {
    request();
    auto v = dataLock()->robot_model_store.getRecv(*this);
    if (v) {
        return *v;
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

} // namespace WEBCFACE_NS
