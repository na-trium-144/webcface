#include "webcface/robot_model.h"
#include <webcface/component_canvas3d.h>
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN

message::Canvas3DComponent Canvas3DComponent::toMessage() const {
    message::Canvas3DComponent cc;
    cc.type = static_cast<int>(this->type_);
    cc.origin_pos = this->origin_.pos();
    cc.origin_rot = this->origin_.rot();
    cc.color = static_cast<int>(this->color_);
    if (this->geometry_) {
        cc.geometry_type = static_cast<int>(this->geometry_->type);
        cc.geometry_properties = this->geometry_->properties;
    }
    if (this->field_base_) {
        cc.field_member = this->field_base_->member_;
        cc.field_field = this->field_base_->field_;
    }
    for (const auto &a : this->angles_) {
        cc.angles.emplace(std::to_string(a.first), a.second);
    }
    return cc;
}
Canvas3DComponent::Canvas3DComponent(const message::Canvas3DComponent &cc)
    : type_(static_cast<Canvas3DComponentType>(cc.type)),
      origin_(cc.origin_pos, cc.origin_rot),
      color_(static_cast<ViewColor>(cc.color)),
      geometry_(cc.geometry_type
                    ? std::make_optional<Geometry>(
                          static_cast<GeometryType>(*cc.geometry_type),
                          cc.geometry_properties)
                    : std::nullopt) {
    for (const auto &a : cc.angles) {
        this->angles_.emplace(std::stoi(a.first), a.second);
    }
}

std::optional<RobotModel> Canvas3DComponent::robotModel() const {
    if (field_base_ && type_ == Canvas3DComponentType::robot_model) {
        return Field{data_w, field_base_->member_, field_base_->field_};
    } else {
        return std::nullopt;
    }
}
Canvas3DComponent &Canvas3DComponent::robotModel(const RobotModel &field) {
    field_base_.emplace(static_cast<FieldBase>(field));
    return *this;
}


Canvas3DComponent &Canvas3DComponent::angles(
    const std::unordered_map<std::string, double> &angles) {
    auto rm = robotModel();
    if (rm) {
        angles_.clear();
        auto model = rm->get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            if (angles.count(j.name.decode())) {
                angles_[ji] = angles.at(j.name.decode());
            }
        }
        return *this;
    } else {
        throw std::invalid_argument("Tried to set Canvas3DComponent::angles "
                                    "but robotModel not defined");
    }
}
Canvas3DComponent &Canvas3DComponent::angles(
    const std::unordered_map<std::wstring, double> &angles) {
    auto rm = robotModel();
    if (rm) {
        angles_.clear();
        auto model = rm->get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            if (angles.count(j.name.decodeW())) {
                angles_[ji] = angles.at(j.name.decodeW());
            }
        }
        return *this;
    } else {
        throw std::invalid_argument("Tried to set Canvas3DComponent::angles "
                                    "but robotModel not defined");
    }
}
Canvas3DComponent &Canvas3DComponent::angle(const std::string &joint_name,
                                            double angle) {
    auto rm = robotModel();
    if (rm) {
        auto model = rm->get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            if (joint_name == j.name.decode()) {
                angles_[ji] = angle;
            }
        }
        return *this;
    } else {
        throw std::invalid_argument("Tried to set Canvas3DComponent::angles "
                                    "but robotModel not defined");
    }
}
Canvas3DComponent &Canvas3DComponent::angle(const std::wstring &joint_name,
                                            double angle) {
    auto rm = robotModel();
    if (rm) {
        auto model = rm->get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            if (joint_name == j.name.decodeW()) {
                angles_[ji] = angle;
            }
        }
        return *this;
    } else {
        throw std::invalid_argument("Tried to set Canvas3DComponent::angles "
                                    "but robotModel not defined");
    }
}

WEBCFACE_NS_END
