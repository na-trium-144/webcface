#include <webcface/canvas3d.h>
#include "client_internal.h"
#include <webcface/member.h>
#include "../message/message.h"
#include "data_buffer.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL EventTarget<Canvas3D>;

Canvas3D::Canvas3D()
    : Field(), EventTarget<Canvas3D>(),
      sb(std::make_shared<Internal::DataSetBuffer<Canvas3DComponent>>()) {}
Canvas3D::Canvas3D(const Field &base)
    : Field(base), EventTarget<Canvas3D>(
                       &this->dataLock()->canvas3d_change_event, *this),
      sb(std::make_shared<Internal::DataSetBuffer<Canvas3DComponent>>(base)) {}
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
void Internal::DataSetBuffer<Canvas3DComponent>::onSync(){
    auto cb = std::make_shared<std::vector<Canvas3DComponentBase>>();
    cb->reserve(components_.size());
    for (std::size_t i = 0; i < components_.size(); i++) {
        cb->push_back(std::move(components_.at(i).lockTmp(
            target_.data_w, target_.name() + "_" + std::to_string(i))));
    }
    target_.setCheck()->canvas3d_store.setSend(target_, cb);
    static_cast<Canvas3D>(target_).triggerEvent(target_);
}

void Canvas3D::request() const {
    auto data = dataLock();
    auto req = data->canvas3d_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Canvas3D>{{}, member_, field_, req}));
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
