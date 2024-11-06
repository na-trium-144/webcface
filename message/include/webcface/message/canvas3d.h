#pragma once
#include "webcface/message/base.h"

WEBCFACE_NS_BEGIN
namespace message {

struct Canvas3DComponent {
    int type = 0;
    std::array<double, 3> origin_pos, origin_rot;
    int color = 0;
    std::optional<int> geometry_type;
    std::vector<double> geometry_properties;
    std::optional<SharedString> field_member, field_field;
    std::map<std::string, double> angles;
    Canvas3DComponent() = default;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("t", type), MSGPACK_NVP("op", origin_pos),
                       MSGPACK_NVP("or", origin_rot), MSGPACK_NVP("c", color),
                       MSGPACK_NVP("gt", geometry_type),
                       MSGPACK_NVP("gp", geometry_properties),
                       MSGPACK_NVP("fm", field_member),
                       MSGPACK_NVP("ff", field_field), MSGPACK_NVP("a", angles))
};
struct Canvas3D : public MessageBase<MessageKind::canvas3d> {
    SharedString field;
    std::map<std::string, std::shared_ptr<Canvas3DComponent>> data_diff;
    std::size_t length = 0;
    Canvas3D() = default;
    Canvas3D(const SharedString &field,
             const std::unordered_map<int, std::shared_ptr<Canvas3DComponent>>
                 &data_diff,
             std::size_t length)
        : field(field), data_diff(), length(length) {
        for (const auto &vc : data_diff) {
            this->data_diff.emplace(std::to_string(vc.first), vc.second);
        }
    }
    Canvas3D(const SharedString &field,
             const std::map<std::string, std::shared_ptr<Canvas3DComponent>>
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
    std::map<std::string, std::shared_ptr<Canvas3DComponent>> data_diff;
    std::size_t length = 0;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::map<std::string, std::shared_ptr<Canvas3DComponent>>
            &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};

} // namespace message
WEBCFACE_NS_END
