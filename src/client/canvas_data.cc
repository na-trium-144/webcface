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

template <typename CComponent, typename CVal, std::size_t v_index,
          typename DecodeF>
CComponent ViewComponent::cDataT(DecodeF decode) const {
    CComponent vcc;
    vcc.type = static_cast<int>(this->type());
    this->text_s.emplace<v_index>(decode(this->text_));
    vcc.text = std::get<v_index>(this->text_s).c_str();
    if (this->on_click_func_) {
        this->on_click_member_s.emplace<v_index>(
            decode(this->on_click_func_->member_));
        this->on_click_field_s.emplace<v_index>(
            decode(this->on_click_func_->field_));
        vcc.on_click_member =
            std::get<v_index>(this->on_click_member_s).c_str();
        vcc.on_click_field = std::get<v_index>(this->on_click_field_s).c_str();
    } else {
        vcc.on_click_member = nullptr;
        vcc.on_click_field = nullptr;
    }
    if (this->text_ref_) {
        this->text_ref_member_s.emplace<v_index>(
            decode(this->text_ref_->member_));
        this->text_ref_field_s.emplace<v_index>(
            decode(this->text_ref_->field_));
        vcc.text_ref_member =
            std::get<v_index>(this->text_ref_member_s).c_str();
        vcc.text_ref_field = std::get<v_index>(this->text_ref_field_s).c_str();
    } else {
        vcc.text_ref_member = nullptr;
        vcc.text_ref_field = nullptr;
    }
    vcc.text_color = static_cast<int>(this->text_color_);
    vcc.bg_color = static_cast<int>(this->bg_color_);
    vcc.min = this->min_.value_or(-DBL_MAX);
    vcc.max = this->max_.value_or(DBL_MAX);
    vcc.step = this->step_.value_or(0);
    std::vector<CVal> options;
    options.reserve(this->option_.size());
    for(const auto &o: this->option_){
        options.push_back(CVal{
            .as_int = o,
            .as_double = o,
            .as_str = o,
        });
    }
    this->options_s.emplace<v_index>(std::move(options));
    vcc.option = std::get<v_index>(this->options_s).data();
    vcc.option_num = this->option_.size();
    return vcc;
}

wcfViewComponent ViewComponent::cData() const {
    return cDataT<wcfViewComponent, wcfMultiVal, 0>(Encoding::decode);
}
wcfViewComponentW ViewComponent::cDataW() const {
    return cDataT<wcfViewComponentW, wcfMultiValW, 1>(Encoding::decodeW);
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
