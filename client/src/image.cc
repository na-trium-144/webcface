#include "webcface/image.h"
#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/member.h"
#include "webcface/internal/client_internal.h"
#include "webcface/common/encoding.h"

WEBCFACE_NS_BEGIN

Image::Image(const Field &base) : Field(base) {}

const Image &Image::tryRequest() const {
    auto req_id = dataLock()->image_store.addReq(member_, field_);
    if (req_id) {
        dataLock()->messagePushReq(message::Req<message::Image>{
            member_, field_, req_id, message::ImageReq{}});
    }
    return *this;
}
const Image &Image::request(std::optional<int> rows, std::optional<int> cols,
                            ImageCompressMode cmp_mode, int quality,
                            std::optional<ImageColorMode> color_mode,
                            std::optional<double> frame_rate) const {
    message::ImageReq req{
        rows,
        cols,
        color_mode ? std::make_optional(
                         static_cast<message::ImageColorMode>(*color_mode))
                   : std::nullopt,
        static_cast<message::ImageCompressMode>(cmp_mode),
        quality,
        frame_rate};
    auto req_id = dataLock()->image_store.addReq(member_, field_, req);
    if (req_id) {
        dataLock()->messagePushReq(
            message::Req<message::Image>{member_, field_, req_id, req});
        this->clear();
    }
    return *this;
}

const Image &Image::set(const ImageFrame &img) const {
    auto data = setCheck();
    data->image_store.setSend(*this, img);
    auto change_event =
        internal::findFromMap2(data->image_change_event.shared_lock().get(),
                               this->member_, this->field_);
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}
const Image &Image::onChange(std::function<void(Image)> callback) const {
    this->tryRequest();
    auto data = dataLock();
    data->image_change_event.lock().get()[this->member_][this->field_] =
        std::make_shared<std::function<void(Image)>>(std::move(callback));
    return *this;
}

std::optional<ImageFrame> Image::tryGet() const {
    this->tryRequest();
    return dataLock()->image_store.getRecv(*this);
}

std::chrono::system_clock::time_point Image::time() const {
    return member().syncTime();
}
const Image &Image::clear() const {
    dataLock()->image_store.clearRecv(*this);
    return *this;
}
const Image &Image::free() const {
    auto req = dataLock()->image_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}
bool Image::exists() const {
    return dataLock()->image_store.getEntry(member_).count(field_);
}

WEBCFACE_NS_END
