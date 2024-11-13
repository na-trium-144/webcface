#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <optional>
#include <vector>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct Canvas2DComponent {
    int type = 0;
    std::array<double, 2> origin_pos;
    double origin_rot;
    int color = 0, fill = 0;
    double stroke_width;
    std::optional<int> geometry_type;
    std::vector<double> properties;
    std::optional<SharedString> on_click_member, on_click_field;
    SharedString text;
    Canvas2DComponent() = default;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("t", type), MSGPACK_NVP("op", origin_pos),
                       MSGPACK_NVP("or", origin_rot), MSGPACK_NVP("c", color),
                       MSGPACK_NVP("f", fill), MSGPACK_NVP("s", stroke_width),
                       MSGPACK_NVP("gt", geometry_type),
                       MSGPACK_NVP("gp", properties),
                       MSGPACK_NVP("L", on_click_member),
                       MSGPACK_NVP("l", on_click_field), MSGPACK_NVP("x", text))
};
struct Canvas2D : public MessageBase<MessageKind::canvas2d> {
    SharedString field;
    double width, height;
    std::map<std::string, std::shared_ptr<Canvas2DComponent>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Canvas2D() = default;
    Canvas2D(
        const SharedString &field, double width, double height,
        std::map<std::string, std::shared_ptr<Canvas2DComponent>> data_diff,
        std::optional<std::vector<SharedString>> data_ids)
        : field(field), width(width), height(height),
          data_diff(std::move(data_diff)), data_ids(std::move(data_ids)) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("w", width),
                       MSGPACK_NVP("h", height), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", data_ids))
};
struct Canvas2DOld : public MessageBase<MessageKind::canvas2d_old> {
    SharedString field;
    double width, height;
    std::map<std::string, std::shared_ptr<Canvas2DComponent>> data_diff;
    std::size_t length;
    Canvas2DOld() = default;
    Canvas2DOld(
        const SharedString &field, double width, double height,
        const std::unordered_map<int, std::shared_ptr<Canvas2DComponent>>
            &data_diff,
        std::size_t length)
        : field(field), width(width), height(height), data_diff(),
          length(length) {
        for (const auto &vc : data_diff) {
            this->data_diff.emplace(std::to_string(vc.first), vc.second);
        }
    }
    Canvas2DOld(const SharedString &field, double width, double height,
                const std::map<std::string, std::shared_ptr<Canvas2DComponent>>
                    &data_diff,
                std::size_t length)
        : field(field), width(width), height(height), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("w", width),
                       MSGPACK_NVP("h", height), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", length))
};
template <>
struct Res<Canvas2DOld>
    : public MessageBase<MessageKind::canvas2d_old + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    double width = 0, height = 0;
    std::map<std::string, std::shared_ptr<Canvas2DComponent>> data_diff;
    std::size_t length;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field, double width,
        double height,
        const std::map<std::string, std::shared_ptr<Canvas2DComponent>>
            &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), width(width), height(height),
          data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("w", width), MSGPACK_NVP("h", height),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};
template <>
struct Res<Canvas2D>
    : public MessageBase<MessageKind::canvas2d + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    double width = 0, height = 0;
    std::map<std::string, std::shared_ptr<Canvas2DComponent>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field, double width,
        double height,
        const std::map<std::string, std::shared_ptr<Canvas2DComponent>>
            &data_diff,
        const std::optional<std::vector<SharedString>> &data_ids)
        : req_id(req_id), sub_field(sub_field), width(width), height(height),
          data_diff(data_diff), data_ids(data_ids) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("w", width), MSGPACK_NVP("h", height),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", data_ids))
};

}
WEBCFACE_NS_END
