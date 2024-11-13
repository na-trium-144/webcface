#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include "webcface/common/image_mode.h"
#include <optional>
#include <vector>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct ImageFrame {
    int width_ = 0, height_ = 0;
    std::shared_ptr<std::vector<unsigned char>> data_;
    ImageColorMode color_mode_ = ImageColorMode::gray;
    ImageCompressMode cmp_mode_ = ImageCompressMode::raw;
};
struct Image : public MessageBase<MessageKind::image>, public ImageFrame {
    SharedString field;
    Image() = default;
    Image(const SharedString &field, const ImageFrame &img)
        : message::MessageBase<MessageKind::image>(), ImageFrame(img),
          field(field) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_),
                       MSGPACK_NVP("h", height_), MSGPACK_NVP("w", width_),
                       MSGPACK_NVP("l", color_mode_),
                       MSGPACK_NVP("p", cmp_mode_))
};

struct ImageReq {
    std::optional<int> rows = std::nullopt, cols = std::nullopt;
    std::optional<ImageColorMode> color_mode = std::nullopt;
    ImageCompressMode cmp_mode = ImageCompressMode::raw;
    int quality = 0;
    std::optional<double> frame_rate = std::nullopt;

    bool operator==(const ImageReq &rhs) const {
        return rows == rhs.rows && cols == rhs.cols &&
               color_mode == rhs.color_mode && cmp_mode == rhs.cmp_mode &&
               quality == rhs.quality;
    }
    bool operator!=(const ImageReq &rhs) const { return !(*this == rhs); }
};
template <typename T>
struct Req;
template <>
struct Req<Image> : public MessageBase<MessageKind::image + MessageKind::req>,
                    public ImageReq {
    SharedString member;
    SharedString field;
    unsigned int req_id;

    Req() = default;
    Req(const SharedString &member, const SharedString &field,
        unsigned int req_id, const ImageReq &ireq)
        : MessageBase<MessageKind::image + MessageKind::req>(), ImageReq(ireq),
          member(member), field(field), req_id(req_id) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("M", member),
                       MSGPACK_NVP("f", field), MSGPACK_NVP("w", cols),
                       MSGPACK_NVP("h", rows), MSGPACK_NVP("l", color_mode),
                       MSGPACK_NVP("p", cmp_mode), MSGPACK_NVP("q", quality),
                       MSGPACK_NVP("r", frame_rate))
};
template <>
struct Res<Image> : public MessageBase<MessageKind::image + MessageKind::res>,
                    public ImageFrame {
    unsigned int req_id = 0;
    SharedString sub_field;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const ImageFrame &img)
        : MessageBase<MessageKind::image + MessageKind::res>(), ImageFrame(img),
          req_id(req_id), sub_field(sub_field) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_), MSGPACK_NVP("w", width_),
                       MSGPACK_NVP("h", height_), MSGPACK_NVP("l", color_mode_),
                       MSGPACK_NVP("p", cmp_mode_))
};

}
WEBCFACE_NS_END
