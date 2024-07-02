#include <webcface/canvas3d.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/encoding/encoding.h"
#include "webcface/internal/event_target_impl.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF EventTarget<Canvas3D>;

Canvas3D::Canvas3D()
    : Field(), EventTarget<Canvas3D>(),
      sb(std::make_shared<internal::DataSetBuffer<Canvas3DComponent>>()) {}
Canvas3D::Canvas3D(const Field &base)
    : Field(base), EventTarget<Canvas3D>(),
      sb(std::make_shared<internal::DataSetBuffer<Canvas3DComponent>>(base)) {
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(
        this->dataLock()->canvas3d_change_event[this->member_][this->field_]);
}
Canvas3D &Canvas3D::init() {
    sb->init();
    return *this;
}
Canvas3D &Canvas3D::sync() {
    sb->sync();
    return *this;
}
Canvas3D &Canvas3D::operator<<(const Canvas3DComponent &cc) {
    sb->add(cc);
    return *this;
}
Canvas3D &Canvas3D::operator<<(Canvas3DComponent &&cc) {
    sb->add(std::move(cc));
    return *this;
}

template <>
void internal::DataSetBuffer<Canvas3DComponent>::onSync() {
    for (std::size_t i = 0; i < components_.size(); i++) {
        components_.at(i).lockTmp(
            target_.data_w,
            target_.field_.u8String() + u8"_" +
                std::u8string(encoding::castToU8(std::to_string(i))));
    }
    target_.setCheck()->canvas3d_store.setSend(
        target_, std::make_shared<std::vector<Canvas3DComponent>>(
                     std::move(components_)));
    static_cast<Canvas3D>(target_).triggerEvent(target_);
}

void Canvas3D::request() const {
    auto data = dataLock();
    auto req = data->canvas3d_store.addReq(member_, field_);
    if (req) {
        data->message_push(message::packSingle(
            message::Req<message::Canvas3D>{{}, member_, field_, req}));
    }
}
void Canvas3D::onAppend() const { request(); }
std::optional<std::vector<Canvas3DComponent>> Canvas3D::tryGet() const {
    auto vb = dataLock()->canvas3d_store.getRecv(*this);
    request();
    if (vb) {
        std::vector<Canvas3DComponent> v((*vb)->size());
        for (std::size_t i = 0; i < (*vb)->size(); i++) {
            v[i] = Canvas3DComponent{(**vb)[i], this->data_w};
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
