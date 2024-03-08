#include <webcface/canvas3d.h>
#include "client_internal.h"
#include <webcface/member.h>
#include "../message/message.h"

namespace WEBCFACE_NS {

Canvas3D::Canvas3D()
    : Field(), EventTarget<Canvas3D>(),
      components(std::make_shared<std::vector<Canvas3DComponent>>()),
      modified(std::make_shared<bool>(false)) {}
Canvas3D::Canvas3D(const Field &base)
    : Field(base), EventTarget<Canvas3D>(
                       &this->dataLock()->canvas3d_change_event, *this),
      components(std::make_shared<std::vector<Canvas3DComponent>>()),
      modified(std::make_shared<bool>(false)) {}
Canvas3D &Canvas3D::init() {
    components->clear();
    *modified = true;
    return *this;
}
Canvas3D &Canvas3D::sync() {
    if (*modified) {
        set();
        *modified = false;
    }
    return *this;
}
void Canvas3D::onDestroy() {
    if (components.use_count() == 1 && data_w.lock() != nullptr &&
        dataLock()->isSelf(member_)) {
        sync();
    }
}
WEBCFACE_DLL Canvas3D &Canvas3D::add(const Canvas3DComponent &cc) {
    components->push_back(cc);
    *modified = true;
    return *this;
}
WEBCFACE_DLL Canvas3D &Canvas3D::add(Canvas3DComponent &&cc) {
    components->push_back(std::move(cc));
    *modified = true;
    return *this;
}

Canvas3D &Canvas3D::set() {
    auto cb = std::make_shared<std::vector<Canvas3DComponentBase>>();
    cb->reserve(components->size());
    for (std::size_t i = 0; i < components->size(); i++) {
        cb->push_back(components->at(i).lockTmp(
            this->data_w, this->name() + "_" + std::to_string(i)));
    }
    setCheck()->canvas3d_store.setSend(*this, cb);
    triggerEvent(*this);
    return *this;
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

} // namespace WEBCFACE_NS
