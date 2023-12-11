#include <webcface/image.h>
#include "client_internal.h"
#include "../message/message.h"

namespace webcface {
Image::Image(const Field &base)
    : Field(base), EventTarget<Image>(&this->dataLock()->image_change_event,
                                      *this) {}

inline void addImageReq(const std::shared_ptr<Internal::ClientData> &data,
                        const std::string &member_, const std::string &field_) {
    auto req = data->image_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Image>{{}, member_, field_, req}));
    }
}

Image &Image::set(const ImageData &img) {
    setCheck()->image_store.setSend(*this, img);
    this->triggerEvent(*this);
    return *this;
}

void Image::onAppend() const { addImageReq(dataLock(), member_, field_); }

std::optional<ImageData> Image::tryGet() const {
    auto v = dataLock()->image_store.getRecv(*this);
    addImageReq(dataLock(), member_, field_);
    if (v) {
        return *v;
    } else {
        return std::nullopt;
    }
}

std::chrono::system_clock::time_point Image::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
Image &Image::free() {
    auto req = dataLock()->image_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace webcface
