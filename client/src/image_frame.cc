#include <webcface/image_frame.h>
#include <webcface/message/message.h>

WEBCFACE_NS_BEGIN

ImageFrame::ImageFrame()
    : size_(), data_(std::make_shared<std::vector<unsigned char>>()),
      color_mode_(ImageColorMode::gray), cmp_mode_(ImageCompressMode::raw) {}
ImageFrame::ImageFrame(const Size &size,
                       const std::shared_ptr<std::vector<unsigned char>> &data,
                       ImageColorMode color_mode, ImageCompressMode cmp_mode)
    : size_(size), data_(data), color_mode_(color_mode), cmp_mode_(cmp_mode) {
    if (cmp_mode == ImageCompressMode::raw &&
        rows() * cols() * channels() != data->size()) {
        throw std::invalid_argument("data size does not match");
    }
}
ImageFrame::ImageFrame(const Size &size, const void *data,
                       ImageColorMode color_mode)
    : size_(size), color_mode_(color_mode), cmp_mode_(ImageCompressMode::raw) {
    data_ = std::make_shared<std::vector<unsigned char>>(
        static_cast<const unsigned char *>(data),
        static_cast<const unsigned char *>(data) +
            rows() * cols() * channels());
}
ImageFrame::ImageFrame(const Size &size, ImageColorMode color_mode)
    : size_(size), color_mode_(color_mode), cmp_mode_(ImageCompressMode::raw) {
    data_ = std::make_shared<std::vector<unsigned char>>(rows() * cols() *
                                                         channels());
}

std::size_t ImageFrame::channels() const {
    switch (color_mode_) {
    case ImageColorMode::gray:
        return 1;
    case ImageColorMode::bgr:
    case ImageColorMode::rgb:
        return 3;
    case ImageColorMode::bgra:
    case ImageColorMode::rgba:
        return 4;
    default:
        throw std::invalid_argument("unknown color format");
    }
}

// bool ImageReq::operator==(const ImageReq &rhs) const {
//     return rows == rhs.rows && cols == rhs.cols &&
//            color_mode == rhs.color_mode && cmp_mode == rhs.cmp_mode &&
//            quality == rhs.quality;
// }

ImageFrame::ImageFrame(const Message::ImageFrame &m)
    : size_(sizeWH(m.width_, m.height_)), data_(m.data_),
      color_mode_(static_cast<ImageColorMode>(m.color_mode_)),
      cmp_mode_(static_cast<ImageCompressMode>(m.cmp_mode_)) {}

Message::ImageFrame ImageFrame::toMessage() const {
    return Message::ImageFrame{width(), height(), data_,
                               static_cast<int>(color_mode_),
                               static_cast<int>(cmp_mode_)};
}

WEBCFACE_NS_END
