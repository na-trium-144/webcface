#include <webcface/robot_model.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {
RobotModel::RobotModel(const Field &base)
    : Field(base), EventTarget<RobotModel>(
                       &this->dataLock()->robot_model_change_event, *this) {}

inline void addRMReq(const std::shared_ptr<Internal::ClientData> &data,
                     const std::string &member_, const std::string &field_) {
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

void RobotModel::onAppend() const { addRMReq(dataLock(), member_, field_); }

std::optional<std::vector<RobotLink>> RobotModel::tryGet() const {
    auto v = dataLock()->robot_model_store.getRecv(*this);
    addRMReq(dataLock(), member_, field_);
    if (v) {
        return *v;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point RobotModel::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
RobotModel &RobotModel::free() {
    auto req = dataLock()->robot_model_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace WEBCFACE_NS
