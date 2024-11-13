#include "webcface/canvas3d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/member.h"
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/encoding/encoding.h"
#include "webcface/internal/component_internal.h"

WEBCFACE_NS_BEGIN

Canvas3D::Canvas3D()
    : Field(), sb(std::make_shared<
                   internal::DataSetBuffer<TemporalCanvas3DComponent>>()) {}
Canvas3D::Canvas3D(const Field &base)
    : Field(base),
      sb(std::make_shared<internal::DataSetBuffer<TemporalCanvas3DComponent>>(
          base)) {}
const Canvas3D &Canvas3D::init() const {
    sb->init();
    return *this;
}
const Canvas3D &Canvas3D::sync() const {
    sb->sync();
    return *this;
}
const Canvas3D &Canvas3D::operator<<(TemporalCanvas3DComponent cc) const {
    sb->add(std::move(cc));
    return *this;
}

template <>
void internal::DataSetBuffer<TemporalCanvas3DComponent>::onSync() {
    std::unordered_map<Canvas3DComponentType, int> idx_next;
    auto data = target_.setCheck();
    auto components_p = std::make_shared<Canvas3DDataBase>();
    components_p->data_ids.reserve(this->components_.size());
    for (std::size_t i = 0; i < components_.size(); i++) {
        std::shared_ptr<internal::Canvas3DComponentData> msg_data =
            components_[i].lockTmp(data, target_.field_, &idx_next);
        components_p->components.emplace(msg_data->id, msg_data);
        components_p->data_ids.push_back(msg_data->id);
    }
    data->canvas3d_store.setSend(target_, components_p);
    std::shared_ptr<std::function<void(Canvas3D)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event =
            data->canvas3d_change_event[target_.member_][target_.field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
const Canvas3D &
Canvas3D::onChange(std::function<void(Canvas3D)> callback) const {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->canvas3d_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Canvas3D)>>(std::move(callback));
    return *this;
}

const Canvas3D &Canvas3D::request() const {
    auto data = dataLock();
    auto req = data->canvas3d_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(message::packSingle(
            message::Req<message::Canvas3D>{{}, member_, field_, req}));
    }
    return *this;
}
std::optional<std::vector<Canvas3DComponent>> Canvas3D::tryGet() const {
    auto vb = dataLock()->canvas3d_store.getRecv(*this);
    request();
    if (vb) {
        std::vector<Canvas3DComponent> v;
        v.reserve((*vb)->data_ids.size());
        for (const auto &id : (*vb)->data_ids) {
            v.emplace_back((*vb)->components.at(id), this->data_w);
        }
        return v;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point Canvas3D::time() const {
    return member().syncTime();
}
const Canvas3D &Canvas3D::free() const {
    auto req = dataLock()->canvas3d_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}
bool Canvas3D::exists() const {
    return dataLock()->canvas3d_store.getEntry(member_).count(field_);
}

WEBCFACE_NS_END
