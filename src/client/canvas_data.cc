#include <webcface/canvas_data.h>
#include <webcface/member.h>
#include <webcface/robot_model.h>
#include "client_internal.h"
#include "webcface/common/field_base.h"

WEBCFACE_NS_BEGIN

ViewComponentBase &
ViewComponent::lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
                       const std::u8string &view_name,
                       std::unordered_map<int, int> *idx_next) {
    auto data = data_w.lock();
    initIdx(idx_next, type_);
    if (on_click_func_tmp) {
        Func on_click{Field{data_w, data->self_member_name},
                      u8"..v" + view_name + u8"/" + id()};
        on_click_func_tmp->lockTo(on_click);
        onClick(on_click);
    }
    if (text_ref_tmp) {
        // if (text_ref_tmp->expired()) {
        Text text_ref{Field{data_w, data->self_member_name},
                      u8"..ir" + view_name + u8"/" + id()};
        text_ref_tmp->lockTo(text_ref);
        if (init_ && !text_ref.tryGet()) {
            text_ref.set(*init_);
        }
        // }
        text_ref_.emplace(static_cast<FieldBase>(text_ref));
    }
    return *this;
}
wcfViewComponent ViewComponent::cData() const {
    wcfViewComponent vcc;
    vcc.type = static_cast<int>(this->type());
    vcc.text = this->text_.empty() ? nullptr : this->text_.c_str();
    if (this->on_click_func_) {
        vcc.on_click_member = this->on_click_func_->member_.c_str();
        vcc.on_click_field = this->on_click_func_->field_.c_str();
    } else {
        vcc.on_click_member = nullptr;
        vcc.on_click_field = nullptr;
    }
    vcc.text_color = static_cast<int>(this->text_color_);
    vcc.bg_color = static_cast<int>(this->bg_color_);
    return vcc;
}

std::optional<Func> ViewComponent::onClick() const {
    if (on_click_func_) {
        // Fieldの中でnullptrは処理してくれるからいいかな
        // assert(data_w.lock() != nullptr && "ClientData not set");
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
ViewComponent &ViewComponent::onClick(const Func &func) {
    on_click_func_.emplace(static_cast<FieldBase>(func));
    return *this;
}

std::optional<Text> ViewComponent::bind() const {
    if (text_ref_) {
        return Field{data_w, text_ref_->member_, text_ref_->field_};
    } else {
        return std::nullopt;
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
            auto j_name_s = Encoding::decode(j.name);
            if (angles.count(j_name_s)) {
                angles_[ji] = angles.at(j_name_s);
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
            auto j_name_s = Encoding::decodeW(j.name);
            if (angles.count(j_name_s)) {
                angles_[ji] = angles.at(j_name_s);
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
            auto j_name_s = Encoding::decode(j.name);
            if (joint_name == j_name_s) {
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
            auto j_name_s = Encoding::decodeW(j.name);
            if (joint_name == j_name_s) {
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
                           const std::u8string &view_name,
                           std::unordered_map<int, int> *idx_next) {
    initIdx(idx_next, type_);
    if (on_click_func_tmp != nullptr) {
        auto data = data_w.lock();
        Func on_click{Field{data_w, data->self_member_name},
                      u8"..c2" + view_name + u8"/" + id()};
        on_click_func_tmp->lockTo(on_click);
        onClick(on_click);
    }
    return *this;
}

std::optional<Func> Canvas2DComponent::onClick() const {
    if (on_click_func_) {
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
Canvas2DComponent &Canvas2DComponent::onClick(const Func &func) {
    on_click_func_.emplace(static_cast<FieldBase>(func));
    return *this;
}


WEBCFACE_NS_END
