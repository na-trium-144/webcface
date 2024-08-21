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
Canvas3D &Canvas3D::init() {
    sb->init();
    return *this;
}
Canvas3D &Canvas3D::sync() {
    sb->sync();
    return *this;
}
Canvas3D &Canvas3D::operator<<(TemporalCanvas3DComponent cc) {
    sb->add(std::move(cc));
    return *this;
}

template <>
void internal::DataSetBuffer<TemporalCanvas3DComponent>::onSync() {
    std::unordered_map<Canvas3DComponentType, int> idx_next;
    auto data = target_.setCheck();
    auto components_p = std::make_shared<
        std::vector<std::shared_ptr<internal::Canvas3DComponentData>>>();
    components_p->reserve(components_.size());
    for (std::size_t i = 0; i < components_.size(); i++) {
        components_p->push_back(
            components_[i].lockTmp(data, target_.field_, &idx_next));
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
Canvas3D &Canvas3D::onChange(std::function<void(Canvas3D)> callback) {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->canvas3d_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Canvas3D)>>(std::move(callback));
    return *this;
}

void Canvas3D::request() const {
    auto data = dataLock();
    auto req = data->canvas3d_store.addReq(member_, field_);
    if (req) {
        data->messagePushOnline(message::packSingle(
            message::Req<message::Canvas3D>{{}, member_, field_, req}));
    }
}
std::optional<std::vector<Canvas3DComponent>> Canvas3D::tryGet() const {
    auto vb = dataLock()->canvas3d_store.getRecv(*this);
    request();
    if (vb) {
        std::vector<Canvas3DComponent> v((*vb)->size());
        std::unordered_map<Canvas3DComponentType, int> idx_next;
        for (std::size_t i = 0; i < (*vb)->size(); i++) {
            v[i] = Canvas3DComponent{(**vb)[i], this->data_w, &idx_next};
        }
        return v;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point Canvas3D::time() const {
    return member().syncTime();
}
Canvas3D &Canvas3D::free() {
    auto req = dataLock()->canvas3d_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

WEBCFACE_NS_END
