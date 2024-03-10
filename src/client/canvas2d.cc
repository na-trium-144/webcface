#include <webcface/canvas2d.h>
#include "client_internal.h"
#include <webcface/member.h>
#include "../message/message.h"

namespace WEBCFACE_NS {

template class WEBCFACE_DLL EventTarget<Canvas2D>;

Canvas2DDataBase
Canvas2DData::lockTmp(std::weak_ptr<Internal::ClientData> data_w,
                      const std::string &field_name) {
    Canvas2DDataBase cb;
    cb.width = this->width;
    cb.height = this->height;
    cb.components.reserve(this->components.size());
    for (std::size_t i = 0; i < this->components.size(); i++) {
        cb.components.emplace_back(std::move(this->components[i].lockTmp(
            data_w, field_name + "_" + std::to_string(i))));
    }
    return cb;
}

Canvas2D::Canvas2D()
    : Field(), EventTarget<Canvas2D>(),
      canvas_data(std::make_shared<Canvas2DData>()),
      modified(std::make_shared<bool>(false)) {}
Canvas2D::Canvas2D(const Field &base)
    : Field(base), EventTarget<Canvas2D>(
                       &this->dataLock()->canvas2d_change_event, *this),
      canvas_data(std::make_shared<Canvas2DData>()),
      modified(std::make_shared<bool>(false)) {}
Canvas2D &Canvas2D::init(double width, double height) {
    canvas_data->components.clear();
    canvas_data->width = width;
    canvas_data->height = height;
    *modified = true;
    return *this;
}
Canvas2D &Canvas2D::sync() {
    if (*modified) {
        set();
        *modified = false;
    }
    return *this;
}
void Canvas2D::onDestroy() {
    if (canvas_data.use_count() == 1 && data_w.lock() != nullptr &&
        dataLock()->isSelf(member_)) {
        sync();
    }
}
WEBCFACE_DLL Canvas2D &Canvas2D::add(const Canvas2DComponent &cc) {
    canvas_data->checkSize();
    canvas_data->components.push_back(cc);
    *modified = true;
    return *this;
}
WEBCFACE_DLL Canvas2D &Canvas2D::add(Canvas2DComponent &&cc) {
    canvas_data->checkSize();
    canvas_data->components.push_back(std::move(cc));
    *modified = true;
    return *this;
}

Canvas2D &Canvas2D::set() {
    canvas_data->checkSize();
    setCheck()->canvas2d_store.setSend(
        *this, std::make_shared<Canvas2DDataBase>(
                   canvas_data->lockTmp(this->data_w, this->name())));
    triggerEvent(*this);
    return *this;
}

void Canvas2D::request() const {
    auto data = dataLock();
    auto req = data->canvas2d_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Canvas2D>{{}, member_, field_, req}));
    }
}
void Canvas2D::onAppend() const { request(); }
std::optional<std::vector<Canvas2DComponent>> Canvas2D::tryGet() const {
    request();
    auto vb = dataLock()->canvas2d_store.getRecv(*this);
    if (vb) {
        std::vector<Canvas2DComponent> v((*vb)->components.size());
        for (std::size_t i = 0; i < (*vb)->components.size(); i++) {
            v[i] = Canvas2DComponent{(*vb)->components[i], this->data_w};
        }
        return v;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point Canvas2D::time() const {
    return member().syncTime();
}
Canvas2D &Canvas2D::free() {
    auto req = dataLock()->canvas2d_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace WEBCFACE_NS
