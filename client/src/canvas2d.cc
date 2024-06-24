#include <webcface/canvas2d.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/event_target_impl.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF EventTarget<Canvas2D>;

Canvas2D::Canvas2D()
    : Field(), EventTarget<Canvas2D>(),
      sb(std::make_shared<Internal::Canvas2DDataBuf>()) {}
Canvas2D::Canvas2D(const Field &base)
    : Field(base), EventTarget<Canvas2D>(),
      sb(std::make_shared<Internal::Canvas2DDataBuf>(base)) {
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(
        this->dataLock()->canvas2d_change_event[this->member_][this->field_]);
}
Canvas2D &Canvas2D::init(double width, double height) {
    sb->init(width, height);
    return *this;
}
Canvas2D &Canvas2D::sync() {
    sb->sync();
    return *this;
}
Canvas2D &Canvas2D::operator<<(const Canvas2DComponent &cc) {
    sb->add(cc);
    return *this;
}
Canvas2D &Canvas2D::operator<<(Canvas2DComponent &&cc) {
    sb->add(std::move(cc));
    return *this;
}

template <>
void Internal::DataSetBuffer<Canvas2DComponent>::onSync() {
    auto c2buf = dynamic_cast<Canvas2DDataBuf *>(this);
    if (!c2buf) {
        throw std::runtime_error("Failed to access Canvas2DDataBuf");
    }
    c2buf->checkSize();

    auto cb = std::make_shared<Canvas2DDataBase>(c2buf->width_, c2buf->height_);
    std::unordered_map<int, int> idx_next;
    for (std::size_t i = 0; i < this->components_.size(); i++) {
        this->components_[i].lockTmp(target_.data_w, target_.field_, &idx_next);
    }
    cb->components = std::move(this->components_);
    target_.setCheck()->canvas2d_store.setSend(target_, cb);
    static_cast<Canvas2D>(target_).triggerEvent(target_);
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
        std::unordered_map<int, int> idx_next;
        for (std::size_t i = 0; i < (*vb)->components.size(); i++) {
            v[i] = Canvas2DComponent{(*vb)->components[i], this->data_w,
                                     &idx_next};
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

WEBCFACE_NS_END
