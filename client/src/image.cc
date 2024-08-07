#include "webcface/image.h"
#include "webcface/member.h"
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"
#include "webcface/encoding/encoding.h"

WEBCFACE_NS_BEGIN

Image::Image(const Field &base) : Field(base) {}

Image &Image::request(std::optional<int> rows, std::optional<int> cols,
                      ImageCompressMode cmp_mode, int quality,
                      std::optional<ImageColorMode> color_mode,
                      std::optional<double> frame_rate) {
    message::ImageReq req{
        rows,
        cols,
        color_mode ? std::make_optional(*color_mode) : std::nullopt,
        cmp_mode,
        quality,
        frame_rate};
    auto req_id = dataLock()->image_store.addReq(member_, field_, req);
    if (req_id) {
        dataLock()->message_push(message::packSingle(
            message::Req<message::Image>{member_, field_, req_id, req}));
        this->clear();
    }
    return *this;
}

inline void addImageReq(const std::shared_ptr<internal::ClientData> &data,
                        const SharedString &member_,
                        const SharedString &field_) {
    auto req = data->image_store.addReq(member_, field_);
    if (req) {
        data->message_push(message::packSingle(
            message::Req<message::Image>{member_, field_, req, {}}));
    }
}

Image &Image::set(const ImageFrame &img) {
    auto data = setCheck();
    data->image_store.setSend(*this, img);
    std::shared_ptr<std::function<void(Image)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event = data->image_change_event[this->member_][this->field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}
Image &Image::onChange(std::function<void(Image)> callback) {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->image_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Image)>>(std::move(callback));
    return *this;
}

std::optional<ImageFrame> Image::tryGet() {
    addImageReq(dataLock(), member_, field_);
    return dataLock()->image_store.getRecv(*this);
}

std::chrono::system_clock::time_point Image::time() const {
    return member().syncTime();
}
Image &Image::clear() {
    dataLock()->image_store.clearRecv(*this);
    return *this;
}
Image &Image::free() {
    auto req = dataLock()->image_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

WEBCFACE_NS_END
