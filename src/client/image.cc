#include <webcface/image.h>
#include <webcface/member.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {
Image::Image(const Field &base)
    : Field(base), EventTarget<Image>(&this->dataLock()->image_change_event,
                                      *this) {}

Image &Image::request(std::optional<int> rows, std::optional<int> cols,
                      Common::ImageCompressMode cmp_mode, int quality,
                      std::optional<Common::ImageColorMode> color_mode,
                      std::optional<double> frame_rate) {
    auto req = dataLock()->image_store.addReq(
        member_, field_,
        {rows, cols, color_mode, cmp_mode, quality, frame_rate});
    if (req) {
        dataLock()->message_queue->push(
            Message::packSingle(Message::Req<Message::Image>{
                member_,
                field_,
                req,
                {rows, cols, color_mode, cmp_mode, quality, frame_rate}}));
        this->clear();
    }
    return *this;
}

inline void addImageReq(const std::shared_ptr<Internal::ClientData> &data,
                        const std::string &member_, const std::string &field_) {
    auto req = data->image_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Image>{member_, field_, req, {}}));
    }
}

Image &Image::set(const ImageFrame &img) {
    this->img = img;
    setCheck()->image_store.setSend(*this, img);
    this->triggerEvent(*this);
    return *this;
}

void Image::onAppend() const { addImageReq(dataLock(), member_, field_); }

std::optional<ImageFrame> Image::tryGet() {
    addImageReq(dataLock(), member_, field_);
    if (this->img) {
        return this->img;
    } else {
        this->img = dataLock()->image_store.getRecv(*this);
        return this->img;
    }
}

#if WEBCFACE_USE_OPENCV
cv::Mat Image::mat() & {
    this->img = dataLock()->image_store.getRecv(*this);
    if (this->img) {
        return this->img->mat();
    }
    return cv::Mat();
}
#endif

std::chrono::system_clock::time_point Image::time() const {
    return member().syncTime();
}
Image &Image::clear() {
    dataLock()->image_store.clearRecv(*this);
    return *this;
}
Image &Image::free() {
    auto req = dataLock()->image_store.unsetRecv(*this);
    this->img = std::nullopt;
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace WEBCFACE_NS
