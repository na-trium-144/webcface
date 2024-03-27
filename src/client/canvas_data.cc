#include <webcface/canvas_data.h>
#include <webcface/member.h>
#include <webcface/robot_model.h>
#include "client_internal.h"

namespace WEBCFACE_NS {

std::optional<RobotModel> Canvas3DComponent::robotModel() const {
    if (field_base_ != std::nullopt &&
        type_ == Canvas3DComponentType::robot_model) {
        return Field{data_w, field_base_->member_, field_base_->field_};
    } else {
        return std::nullopt;
    }
}
Canvas3DComponent &Canvas3DComponent::robotModel(const RobotModel &field) {
    field_base_.emplace(field.member().name(), field.name());
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
            if (angles.count(j.name)) {
                angles_[ji] = angles.at(j.name);
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
            if (joint_name == j.name) {
                angles_[ji] = angle;
            }
        }
        return *this;
    } else {
        throw std::invalid_argument("Tried to set Canvas3DComponent::angles "
                                    "but robotModel not defined");
    }
}

Canvas2DComponentBase &
Canvas2DComponent::lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
                           const std::string &view_name, int *func_next) {
    if (on_click_func_tmp != nullptr) {
        auto data = data_w.lock();
        Func on_click{Field{data_w, data->self_member_name},
                      ".c2" + view_name + "." + std::to_string((*func_next)++)};
        on_click_func_tmp->lockTo(on_click);
        on_click.hidden(true);
        onClick(on_click);
    }
    return *this;
}

std::optional<Func> Canvas2DComponent::onClick() const {
    if (on_click_func_ != std::nullopt) {
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
Canvas2DComponent &Canvas2DComponent::onClick(const Func &func) {
    on_click_func_.emplace(func.member().name(), func.name());
    return *this;
}


} // namespace WEBCFACE_NS
