#pragma once
#include "webcface/message/robot_model.h"
#include "webcface/robot_link.h"

WEBCFACE_NS_BEGIN
namespace internal {
struct TemporalRobotJointData {
    TemporalRobotJointData() = default;

    SharedString name;
    SharedString parent_name;
    RobotJointType type;
    Transform origin;
    double angle = 0;
};
struct RobotLinkData : message::RobotLink {
    RobotLinkData() = default;
    /*!
     * jointのparentをid指定から名前指定に変換
     */
    explicit RobotLinkData(const message::RobotLink &data,
                           const std::vector<std::shared_ptr<RobotLinkData>> &links)
        : message::RobotLink(data) {
        if (this->joint_parent >= 0 &&
            static_cast<std::size_t>(this->joint_parent) < links.size()) {
            parent_name = links[this->joint_parent]->name;
            parent_ptr = links[this->joint_parent];
        }
    }
    /*!
     * jointのparentを名前指定からid指定に変換
     * (message送信用)
     */
    void lockJoints(const std::vector<SharedString> &link_names);

    SharedString parent_name;
    std::weak_ptr<RobotLinkData> parent_ptr;
};
} // namespace internal

WEBCFACE_NS_END
