#include "webcface/robot_link.h"
#include "webcface/message/message.h"
#include "webcface/internal/robot_link_internal.h"

WEBCFACE_NS_BEGIN

RobotJoint::RobotJoint() : temp_data(), msg_data() {}
RobotJoint::RobotJoint(const std::shared_ptr<internal::RobotLinkData> &msg_data)
    : temp_data(), msg_data(msg_data) {}
RobotJoint::RobotJoint(const SharedString &name,
                       const SharedString &parent_name, RobotJointType type,
                       const Transform &origin, double angle)
    : temp_data(std::make_unique<internal::TemporalRobotJointData>()),
      msg_data() {
    temp_data->name = name;
    temp_data->parent_name = parent_name;
    temp_data->type = type;
    temp_data->origin = origin;
    temp_data->angle = angle;
}
RobotJoint::~RobotJoint() = default;
RobotJoint::RobotJoint(const RobotJoint &other) : msg_data(other.msg_data) {
    if (other.temp_data) {
        this->temp_data = std::make_unique<internal::TemporalRobotJointData>(
            *other.temp_data);
    }
}
RobotJoint &RobotJoint::operator=(const RobotJoint &other) {
    if (this != &other) {
        this->msg_data = other.msg_data;
        if (other.temp_data) {
            this->temp_data =
                std::make_unique<internal::TemporalRobotJointData>(
                    *other.temp_data);
        }
    }
    return *this;
}

RobotLink::RobotLink() : msg_data() {}
RobotLink::RobotLink(const std::shared_ptr<internal::RobotLinkData> &msg_data)
    : msg_data(msg_data) {}
RobotLink::~RobotLink() = default;
RobotLink::RobotLink(const SharedString &name, const RobotJoint &joint,
                     const Geometry &geometry, ViewColor color)
    : msg_data(std::make_shared<internal::RobotLinkData>()) {
    msg_data->name = name;
    msg_data->joint_parent = -1;
    if (joint.temp_data) {
        msg_data->joint_name = joint.temp_data->name;
        msg_data->parent_name = joint.temp_data->parent_name;
        msg_data->joint_type = static_cast<int>(joint.temp_data->type);
        msg_data->joint_origin_pos = joint.temp_data->origin.pos();
        msg_data->joint_origin_rot = joint.temp_data->origin.rot();
        msg_data->joint_angle = joint.temp_data->angle;
    }
    if (joint.msg_data) {
        // 別のmodelからlinkデータをコピーしようとした?
        // todo
        throw std::runtime_error("Unexpected joint data (msg_data is not null) "
                                 "in constructor of RobotLink");
    }
    msg_data->geometry_type = static_cast<int>(geometry.type);
    msg_data->geometry_properties = geometry.properties;
    msg_data->color = static_cast<int>(color);
}

std::shared_ptr<internal::RobotLinkData>
RobotLink::lockJoints(const std::vector<SharedString> &link_names) const {
    if (msg_data) {
        msg_data->lockJoints(link_names);
    } else {
        throw std::runtime_error("Uninitialized link passed to RobotModel");
    }
    return msg_data;
}
void internal::RobotLinkData::lockJoints(
    const std::vector<SharedString> &link_names) {
    auto parent_it =
        std::find(link_names.begin(), link_names.end(), this->parent_name);
    if (parent_it != link_names.end()) {
        this->joint_parent =
            static_cast<int>(std::distance(link_names.begin(), parent_it));
    }
}

const std::string &RobotJoint::name() const {
    if (msg_data) {
        return msg_data->joint_name.decode();
    } else if (temp_data) {
        return temp_data->name.decode();
    } else {
        return SharedString::emptyStr();
    }
}
const std::wstring &RobotJoint::nameW() const {
    if (msg_data) {
        return msg_data->joint_name.decodeW();
    } else if (temp_data) {
        return temp_data->name.decodeW();
    } else {
        return SharedString::emptyStrW();
    }
}
std::optional<RobotLink> RobotJoint::parent() const {
    if (msg_data && !msg_data->parent_ptr.expired()) {
        return RobotLink(msg_data->parent_ptr.lock());
    } else {
        return std::nullopt;
    }
}
RobotJointType RobotJoint::type() const {
    if (msg_data) {
        return static_cast<RobotJointType>(msg_data->joint_type);
    } else if (temp_data) {
        return temp_data->type;
    } else {
        return RobotJointType::fixed_absolute;
    }
}
Transform RobotJoint::origin() const {
    if (msg_data) {
        return Transform(msg_data->joint_origin_pos,
                         msg_data->joint_origin_rot);
    } else if (temp_data) {
        return temp_data->origin;
    } else {
        return Transform();
    }
}
double RobotJoint::angle() const {
    if (msg_data) {
        return msg_data->joint_angle;
    } else if (temp_data) {
        return temp_data->angle;
    } else {
        return 0;
    }
}

const std::string &RobotLink::name() const {
    if (msg_data) {
        return msg_data->name.decode();
    } else {
        return SharedString::emptyStr();
    }
}
const std::wstring &RobotLink::nameW() const {
    if (msg_data) {
        return msg_data->name.decodeW();
    } else {
        return SharedString::emptyStrW();
    }
}
RobotJoint RobotLink::joint() const { return RobotJoint(msg_data); }
std::optional<Geometry> RobotLink::geometry() const {
    if (msg_data &&
        msg_data->geometry_type != static_cast<int>(GeometryType::none)) {
        return Geometry(static_cast<GeometryType>(msg_data->geometry_type),
                        msg_data->geometry_properties);
    } else {
        return std::nullopt;
    }
}
ViewColor RobotLink::color() const {
    if (msg_data) {
        return static_cast<ViewColor>(msg_data->color);
    } else {
        return ViewColor::inherit;
    }
}

WEBCFACE_NS_END
