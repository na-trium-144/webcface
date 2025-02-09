#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>
#include <array>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct Canvas3DComponentData {
    int type = 0;
    std::array<double, 3> origin_pos, origin_rot;
    int color = 0;
    std::optional<int> geometry_type;
    std::vector<double> geometry_properties;
    std::optional<SharedString> field_member, field_field;
    std::map<std::string, double> angles;
    double scale_x = 1, scale_y = 1;
    Canvas3DComponentData() = default;
    bool operator==(const Canvas3DComponentData &other) const {
        return type == other.type && origin_pos == other.origin_pos &&
               origin_rot == other.origin_rot && color == other.color &&
               geometry_type == other.geometry_type &&
               geometry_properties == other.geometry_properties &&
               field_member == other.field_member &&
               field_field == other.field_field && angles == other.angles &&
               scale_x == other.scale_x && scale_y == other.scale_y;
    }
    bool operator!=(const Canvas3DComponentData &other) const {
        return !(*this == other);
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("t", type), MSGPACK_NVP("op", origin_pos),
                       MSGPACK_NVP("or", origin_rot), MSGPACK_NVP("c", color),
                       MSGPACK_NVP("gt", geometry_type),
                       MSGPACK_NVP("gp", geometry_properties),
                       MSGPACK_NVP("fm", field_member),
                       MSGPACK_NVP("ff", field_field), MSGPACK_NVP("a", angles),
                       MSGPACK_NVP("sx", scale_x), MSGPACK_NVP("sy", scale_y))
};
struct Canvas3DData {
    std::map<std::string, std::shared_ptr<Canvas3DComponentData>> components;
    std::vector<SharedString> data_ids;
    Canvas3DData() = default;
};
struct Canvas3D : public MessageBase<MessageKind::canvas3d> {
    SharedString field;
    std::map<std::string, std::shared_ptr<Canvas3DComponentData>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Canvas3D() = default;
    Canvas3D(
        const SharedString &field,
        std::map<std::string, std::shared_ptr<Canvas3DComponentData>> data_diff,
        std::optional<std::vector<SharedString>> data_ids)
        : field(field), data_diff(std::move(data_diff)),
          data_ids(std::move(data_ids)) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", data_ids))
};
struct Canvas3DOld : public MessageBase<MessageKind::canvas3d_old> {
    SharedString field;
    std::map<std::string, std::shared_ptr<Canvas3DComponentData>> data_diff;
    std::size_t length = 0;
    Canvas3DOld() = default;
    Canvas3DOld(
        const SharedString &field,
        const std::unordered_map<int, std::shared_ptr<Canvas3DComponentData>>
            &data_diff,
        std::size_t length)
        : field(field), data_diff(), length(length) {
        for (const auto &vc : data_diff) {
            this->data_diff.emplace(std::to_string(vc.first), vc.second);
        }
    }
    Canvas3DOld(
        const SharedString &field,
        const std::map<std::string, std::shared_ptr<Canvas3DComponentData>>
            &data_diff,
        std::size_t length)
        : field(field), data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", length))
};
template <>
struct Res<Canvas3D>
    : public MessageBase<MessageKind::canvas3d + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::map<std::string, std::shared_ptr<Canvas3DComponentData>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::map<std::string, std::shared_ptr<Canvas3DComponentData>>
            &data_diff,
        const std::optional<std::vector<SharedString>> &data_ids)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          data_ids(data_ids) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", data_ids))
};
template <>
struct Res<Canvas3DOld>
    : public MessageBase<MessageKind::canvas3d_old + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::map<std::string, std::shared_ptr<Canvas3DComponentData>> data_diff;
    std::size_t length = 0;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::map<std::string, std::shared_ptr<Canvas3DComponentData>>
            &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};

} // namespace message
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::Canvas3D)
WEBCFACE_MESSAGE_FMT(webcface::message::Res<webcface::message::Canvas3D>)
WEBCFACE_MESSAGE_FMT(webcface::message::Entry<webcface::message::Canvas3D>)
WEBCFACE_MESSAGE_FMT(webcface::message::Req<webcface::message::Canvas3D>)
WEBCFACE_MESSAGE_FMT(webcface::message::Canvas3DOld)
WEBCFACE_MESSAGE_FMT(webcface::message::Res<webcface::message::Canvas3DOld>)
WEBCFACE_MESSAGE_FMT(webcface::message::Entry<webcface::message::Canvas3DOld>)
WEBCFACE_MESSAGE_FMT(webcface::message::Req<webcface::message::Canvas3DOld>)
