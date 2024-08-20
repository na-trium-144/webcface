#include "webcface/canvas2d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/member.h"
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"

WEBCFACE_NS_BEGIN

Canvas2D::Canvas2D()
    : Field(), sb(std::make_shared<internal::Canvas2DDataBuf>()) {}
Canvas2D::Canvas2D(const Field &base)
    : Field(base), sb(std::make_shared<internal::Canvas2DDataBuf>(base)) {}
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
void internal::DataSetBuffer<Canvas2DComponent>::onSync() {
    auto c2buf = dynamic_cast<Canvas2DDataBuf *>(this);
    if (!c2buf) {
        throw std::runtime_error("Failed to access Canvas2DDataBuf");
    }
    c2buf->checkSize();

    auto cb = std::make_shared<Canvas2DDataBase>(c2buf->width_, c2buf->height_);
    std::unordered_map<int, int> idx_next;
    auto data = target_.setCheck();
    for (std::size_t i = 0; i < this->components_.size(); i++) {
        this->components_[i].lockTmp(data, target_.field_, &idx_next);
    }
    cb->components = std::move(this->components_);
    data->canvas2d_store.setSend(target_, cb);
    std::shared_ptr<std::function<void(Canvas2D)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event =
            data->canvas2d_change_event[target_.member_][target_.field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
Canvas2D &Canvas2D::onChange(std::function<void(Canvas2D)> callback) {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->canvas2d_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Canvas2D)>>(std::move(callback));
    return *this;
}

void Canvas2D::request() const {
    auto data = dataLock();
    auto req = data->canvas2d_store.addReq(member_, field_);
    if (req) {
        data->messagePushOnline(message::packSingle(
            message::Req<message::Canvas2D>{{}, member_, field_, req}));
    }
}
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
