#include <webcface/robot_link.h>
#include <webcafce/message/message.h>
WEBCFACE_NS_BEGIN
Message::RobotLink
RobotLink::toMessage(const std::vector<SharedString> &link_names) const {
    return Message::RobotLink{
        name,
        joint.name,
        std::distance(
            link_names.begin(),
            std::find(link_names.begin(), link_names.end(), joint.parent_name)),
        joint.type,
        joint.origin.pos(),
        joint.origin.rot(),
        joint.angle,
        geometry.type,
        geometry.properties,
        color};
}
RobotLink::RobotLink(const Message::RobotLink &m,
                     const std::vector<SharedString> &link_names)
    : RobotLink(m.name,
                {m.joint_name,
                 m.joint_parent < link_names.size()
                     ? link_names.at(m.joint_parent)
                     : nullptr,
                 m.joint_type,
                 {m.joint_origin_pos, m.joint_origin_rot},
                 m.joint_angle},
                Geometry{m.geometry_type, m.geometry_properties}, m.color) {}
WEBCFACE_NS_END