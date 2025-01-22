#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <vector>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct RobotLink {
    SharedString name;
    SharedString joint_name;
    int joint_parent;
    int joint_type;
    std::array<double, 3> joint_origin_pos, joint_origin_rot;
    double joint_angle = 0;
    int geometry_type;
    std::vector<double> geometry_properties;
    int color;
    RobotLink() = default;
    RobotLink(const SharedString &name, const SharedString &joint_name,
              int joint_parent, int joint_type,
              const std::array<double, 3> &joint_origin_pos,
              const std::array<double, 3> &joint_origin_rot, double joint_angle,
              int geometry_type, const std::vector<double> &geometry_properties,
              int color)
        : name(name), joint_name(joint_name), joint_parent(joint_parent),
          joint_type(joint_type), joint_origin_pos(joint_origin_pos),
          joint_origin_rot(joint_origin_rot), joint_angle(joint_angle),
          geometry_type(geometry_type),
          geometry_properties(geometry_properties), color(color) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("jn", joint_name),
                       MSGPACK_NVP("jp", joint_parent),
                       MSGPACK_NVP("jt", joint_type),
                       MSGPACK_NVP("js", joint_origin_pos),
                       MSGPACK_NVP("jr", joint_origin_rot),
                       MSGPACK_NVP("ja", joint_angle),
                       MSGPACK_NVP("gt", geometry_type),
                       MSGPACK_NVP("gp", geometry_properties),
                       MSGPACK_NVP("c", color))
};
struct RobotModel : public MessageBase<MessageKind::robot_model> {
    SharedString field;
    std::vector<std::shared_ptr<RobotLink>> data;
    RobotModel() = default;
    RobotModel(const SharedString &field,
               const std::vector<std::shared_ptr<RobotLink>> &data)
        : field(field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};

template <>
struct Res<RobotModel>
    : public MessageBase<MessageKind::robot_model + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::vector<std::shared_ptr<RobotLink>> data;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::vector<std::shared_ptr<RobotLink>> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data))
};

} // namespace message
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::RobotModel)
WEBCFACE_MESSAGE_FMT(webcface::message::Res<webcface::message::RobotModel>)
WEBCFACE_MESSAGE_FMT(webcface::message::Entry<webcface::message::RobotModel>)
WEBCFACE_MESSAGE_FMT(webcface::message::Req<webcface::message::RobotModel>)
