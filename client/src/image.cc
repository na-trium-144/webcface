#include <webcface/image.h>
#include <webcface/member.h>
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"
#include "webcface/encoding/encoding.h"
#include "webcface/internal/event_target_impl.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF EventTarget<Image>;

Image::Image(const Field &base) : Field(base), EventTarget<Image>() {
    std::lock_guard lock(this->dataLock()->event_m);
    this->setCL(
        this->dataLock()->image_change_event[this->member_][this->field_]);
}

Image &Image::request(std::optional<int> rows, std::optional<int> cols,
                      ImageCompressMode cmp_mode, int quality,
                      std::optional<ImageColorMode> color_mode,
                      std::optional<double> frame_rate) {
    message::ImageReq req{
        rows,
        cols,
        color_mode ? std::make_optional(static_cast<int>(*color_mode))
                   : std::nullopt,
        static_cast<int>(cmp_mode),
        quality,
        frame_rate};
    auto req_id = dataLock()->image_store.addReq(member_, field_, req);
    if (req_id) {
        dataLock()->message_queue->push(message::packSingle(
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
        data->message_queue->push(message::packSingle(
            message::Req<message::Image>{member_, field_, req, {}}));
    }
}

Image &Image::set(const ImageFrame &img) {
    setCheck()->image_store.setSend(*this, img);
    this->triggerEvent(*this);
    return *this;
}

void Image::onAppend() const { addImageReq(dataLock(), member_, field_); }

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
