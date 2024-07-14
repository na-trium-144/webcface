#include "webcface/robot_link.h"
#include "webcface/message/message.h"
WEBCFACE_NS_BEGIN
message::RobotLink
RobotLink::toMessage(const std::vector<SharedString> &link_names) const {
    auto parent_it =
        std::find(link_names.begin(), link_names.end(), joint.parent_name);
    return message::RobotLink{
        name,
        joint.name,
        (parent_it == link_names.end()
             ? -1
             : static_cast<int>(std::distance(link_names.begin(), parent_it))),
        static_cast<int>(joint.type),
        joint.origin.pos(),
        joint.origin.rot(),
        joint.angle,
        static_cast<int>(geometry.type),
        geometry.properties,
        static_cast<int>(color)};
}
RobotLink::RobotLink(const message::RobotLink &m,
                     const std::vector<SharedString> &link_names)
    : RobotLink(m.name,
                {m.joint_name,
                 (m.joint_parent >= 0 && static_cast<std::size_t>(
                                             m.joint_parent) < link_names.size()
                      ? link_names.at(m.joint_parent)
                      : nullptr),
                 static_cast<RobotJointType>(m.joint_type),
                 {m.joint_origin_pos, m.joint_origin_rot},
                 m.joint_angle},
                Geometry{static_cast<GeometryType>(m.geometry_type),
                         m.geometry_properties},
                static_cast<ViewColor>(m.color)) {}
WEBCFACE_NS_END
