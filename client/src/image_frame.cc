#include <cstddef>
#include <stdexcept>

#include "webcface/image_frame.h"
#include "webcface/common/internal/message/image.h"

WEBCFACE_NS_BEGIN

ImageFrame::ImageFrame()
    : size_(), data_(std::make_shared<std::vector<unsigned char>>()),
      color_mode_(ImageColorMode::gray), cmp_mode_(ImageCompressMode::raw) {}
ImageFrame::ImageFrame(const ImageFrame &m)
    : size_(m.size_), data_(m.data_), color_mode_(m.color_mode_),
      cmp_mode_(m.cmp_mode_) {}
ImageFrame &ImageFrame::operator=(const ImageFrame &m) {
    if (this != &m) {
        size_ = m.size_;
        data_ = m.data_;
        color_mode_ = m.color_mode_;
        cmp_mode_ = m.cmp_mode_;
    }
    return *this;
}
ImageFrame::ImageFrame(ImageFrame &&m) noexcept
    : size_(m.size_), data_(std::make_shared<std::vector<unsigned char>>()),
      color_mode_(m.color_mode_), cmp_mode_(m.cmp_mode_) {
    m.size_ = Size();
    m.data_.swap(data_);
}
ImageFrame &ImageFrame::operator=(ImageFrame &&m) noexcept {
    if (this != &m) {
        size_ = m.size_;
        m.size_ = Size();
        data_.swap(m.data_);
        color_mode_ = m.color_mode_;
        cmp_mode_ = m.cmp_mode_;
    }
    return *this;
}

ImageFrame::ImageFrame(const Size &size,
                       const std::shared_ptr<std::vector<unsigned char>> &data,
                       ImageColorMode color_mode, ImageCompressMode cmp_mode)
    : size_(size), data_(data), color_mode_(color_mode), cmp_mode_(cmp_mode) {
    if (cmp_mode == ImageCompressMode::raw &&
        rows() * cols() * channels() != static_cast<int>(data->size())) {
        throw std::invalid_argument("data size does not match");
    }
}
ImageFrame::ImageFrame(const Size &size, const void *data,
                       ImageColorMode color_mode)
    : size_(size), color_mode_(color_mode), cmp_mode_(ImageCompressMode::raw) {
    data_ = std::make_shared<std::vector<unsigned char>>(
        static_cast<const unsigned char *>(data),
        static_cast<const unsigned char *>(data) +
            static_cast<ptrdiff_t>(rows() * cols() * channels()));
}
ImageFrame::ImageFrame(const Size &size, ImageColorMode color_mode)
    : size_(size), color_mode_(color_mode), cmp_mode_(ImageCompressMode::raw) {
    data_ = std::make_shared<std::vector<unsigned char>>(rows() * cols() *
                                                         channels());
}

int ImageFrame::channels() const {
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

ImageFrame::ImageFrame(const message::ImageFrame &m)
    : size_(sizeWH(m.width_, m.height_)), data_(m.data_),
      color_mode_(static_cast<ImageColorMode>(m.color_mode_)),
      cmp_mode_(static_cast<ImageCompressMode>(m.cmp_mode_)) {}

message::ImageFrame ImageFrame::toMessage() const {
    return message::ImageFrame{
        width(), height(), data_,
        static_cast<message::ImageColorMode>(color_mode_),
        static_cast<message::ImageCompressMode>(cmp_mode_)};
}

WEBCFACE_NS_END
