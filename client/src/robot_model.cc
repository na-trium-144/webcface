#include "webcface/robot_model.h"
#include "webcface/member.h"
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/robot_link_internal.h"

WEBCFACE_NS_BEGIN

RobotModel::RobotModel()
    : Field(), sb(std::make_shared<internal::DataSetBuffer<RobotLink>>()) {}

RobotModel::RobotModel(const Field &base)
    : Field(base),
      sb(std::make_shared<internal::DataSetBuffer<RobotLink>>(base)) {}

TemporalCanvas3DComponent RobotModel::toComponent3D() const {
    return TemporalCanvas3DComponent{Canvas3DComponentType::robot_model}
        .robotModel(*this);
}

const RobotModel &RobotModel::init() const {
    sb->init();
    return *this;
}
const RobotModel &RobotModel::sync() const {
    sb->sync();
    return *this;
}
template <>
void internal::DataSetBuffer<RobotLink>::onSync() {
    auto ls = std::make_shared<
        std::vector<std::shared_ptr<internal::RobotLinkData>>>();
    std::vector<SharedString> link_names;
    link_names.reserve(components_.size());
    for (const auto &ln : components_) {
        auto ln_msg = ln.lockJoints(link_names);
        link_names.push_back(ln_msg->name);
        ls->push_back(ln_msg);
    }
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
const RobotModel &
RobotModel::onChange(std::function<void(RobotModel)> callback) const {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->robot_model_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(RobotModel)>>(std::move(callback));
    return *this;
}

const RobotModel &RobotModel::operator<<(RobotLink vc) const {
    sb->add(std::move(vc));
    return *this;
}

const RobotModel &RobotModel::request() const {
    auto data = dataLock();
    auto req = data->robot_model_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(message::packSingle(
            message::Req<message::RobotModel>{{}, member_, field_, req}));
    }
    return *this;
}

const RobotModel &RobotModel::set(const std::vector<RobotLink> &v) const {
    sb->set(v); // set()のなかでchange_eventは呼ばれる
    return *this;
}

std::optional<std::vector<RobotLink>> RobotModel::tryGet() const {
    request();
    auto v = dataLock()->robot_model_store.getRecv(*this);
    if (v) {
        std::vector<RobotLink> links;
        for (const auto &ln_msg : **v) {
            links.emplace_back(ln_msg);
        }
        return links;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point RobotModel::time() const {
    return member().syncTime();
}
const RobotModel &RobotModel::free() const {
    auto req = dataLock()->robot_model_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}
bool RobotModel::exists() const {
    return dataLock()->robot_model_store.getEntry(member_).count(field_);
}

WEBCFACE_NS_END
