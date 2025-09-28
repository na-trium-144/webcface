#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/canvas2d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/member.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/component_internal.h"
#include "webcface/exception.h"

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
        throw SanityError("Failed to access Canvas2DDataBuf");
    }
    c2buf->checkSize();

    auto cb =
        std::make_shared<message::Canvas2DData>(c2buf->width_, c2buf->height_);
    std::unordered_map<Canvas2DComponentType, int> idx_next;
    auto data = target_.setCheck();
    cb->data_ids.reserve(this->components_.size());
    for (std::size_t i = 0; i < this->components_.size(); i++) {
        std::shared_ptr<internal::TemporalCanvas2DComponentData> msg_data =
            this->components_[i].lockTmp(data, target_.field_, &idx_next);
        cb->components.emplace(
            std::string(msg_data->id.u8StringView()),
            std::static_pointer_cast<message::Canvas2DComponentData>(msg_data));
        cb->data_ids.push_back(msg_data->id);
    }
    data->canvas2d_store.setSend(target_, cb);
    auto change_event =
        internal::findFromMap2(data->canvas2d_change_event.shared_lock().get(),
                               target_.member_, target_.field_);
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
const Canvas2D &
Canvas2D::onChange(std::function<void(Canvas2D)> callback) const {
    this->request();
    auto data = dataLock();
    data->canvas2d_change_event.lock().get()[this->member_][this->field_] =
        std::make_shared<std::function<void(Canvas2D)>>(std::move(callback));
    return *this;
}

const Canvas2D &Canvas2D::request() const {
    auto data = dataLock();
    auto req = data->canvas2d_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(
            message::Req<message::Canvas2D>{{}, member_, field_, req});
    }
    return *this;
}
std::optional<std::vector<Canvas2DComponent>> Canvas2D::tryGet() const {
    request();
    auto vb = dataLock()->canvas2d_store.getRecv(*this);
    if (vb) {
        std::vector<Canvas2DComponent> v;
        v.reserve((*vb)->data_ids.size());
        for (const auto &id : (*vb)->data_ids) {
            v.emplace_back((*vb)->components.find(id.u8StringView())->second,
                           this->data_w, id);
        }
        return v;
    } else {
        return std::nullopt;
    }
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
