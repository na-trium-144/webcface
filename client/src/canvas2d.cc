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
const Canvas2D &Canvas2D::init(double width, double height) const {
    sb->init(width, height);
    return *this;
}
const Canvas2D &Canvas2D::sync() const {
    sb->sync();
    return *this;
}
const Canvas2D &Canvas2D::operator<<(TemporalCanvas2DComponent cc) const {
    sb->add(std::move(cc));
    return *this;
}

template <>
void internal::DataSetBuffer<TemporalCanvas2DComponent>::onSync() {
    auto c2buf = dynamic_cast<Canvas2DDataBuf *>(this);
    if (!c2buf) {
        throw std::runtime_error("Failed to access Canvas2DDataBuf");
    }
    c2buf->checkSize();

    auto cb = std::make_shared<Canvas2DDataBase>(c2buf->width_, c2buf->height_);
    std::unordered_map<Canvas2DComponentType, int> idx_next;
    auto data = target_.setCheck();
    cb->components.reserve(this->components_.size());
    for (std::size_t i = 0; i < this->components_.size(); i++) {
        cb->components.push_back(
            this->components_[i].lockTmp(data, target_.field_, &idx_next));
    }
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
const Canvas2D &
Canvas2D::onChange(std::function<void(Canvas2D)> callback) const {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->canvas2d_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Canvas2D)>>(std::move(callback));
    return *this;
}

const Canvas2D &Canvas2D::request() const {
    auto data = dataLock();
    auto req = data->canvas2d_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(message::packSingle(
            message::Req<message::Canvas2D>{{}, member_, field_, req}));
    }
    return *this;
}
std::optional<std::vector<Canvas2DComponent>> Canvas2D::tryGet() const {
    request();
    auto vb = dataLock()->canvas2d_store.getRecv(*this);
    if (vb) {
        std::vector<Canvas2DComponent> v((*vb)->components.size());
        std::unordered_map<Canvas2DComponentType, int> idx_next;
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
const Canvas2D &Canvas2D::free() const {
    auto req = dataLock()->canvas2d_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}
bool Canvas2D::exists() const {
    return dataLock()->canvas2d_store.getEntry(member_).count(field_);
}

WEBCFACE_NS_END
