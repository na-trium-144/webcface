#include <webcface/component_view.h>
#include <webcface/member.h>
#include "webcface/internal/client_internal.h"
#include "webcface/field.h"
#include "webcface/encoding/encoding.h"
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF IdBase<ViewComponentType>;

ViewComponent &
ViewComponent::lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
                       const SharedString &view_name,
                       std::unordered_map<int, int> *idx_next) {
    auto data = data_w.lock();
    initIdx(idx_next, type_);
    if (on_click_func_tmp) {
        Func on_click{Field{data_w, data->self_member_name},
                      SharedString(u8"..v" + view_name.u8String() + u8"/" +
                                   std::u8string(Encoding::castToU8(id())))};
        on_click_func_tmp->lockTo(on_click);
        onClick(on_click);
    }
    if (text_ref_tmp) {
        // if (text_ref_tmp->expired()) {
        Text text_ref{Field{data_w, data->self_member_name},
                      SharedString(u8"..ir" + view_name.u8String() + u8"/" +
                                   std::u8string(Encoding::castToU8(id())))};
        text_ref_tmp->lockTo(text_ref);
        if (init_ && !text_ref.tryGet()) {
            text_ref.set(*init_);
        }
        // }
        text_ref_.emplace(static_cast<FieldBase>(text_ref));
    }
    return *this;
}

template <typename CComponent, typename CVal, std::size_t v_index>
CComponent ViewComponent::cDataT() const {
    CComponent vcc;
    vcc.type = static_cast<int>(this->type());
    if constexpr (v_index == 0) {
        vcc.text = this->text_.decode().c_str();
    } else {
        vcc.text = this->text_.decodeW().c_str();
    }
    if (this->on_click_func_) {
        if constexpr (v_index == 0) {
            vcc.on_click_member =
                this->on_click_func_->member_.decode().c_str();
            vcc.on_click_field = this->on_click_func_->field_.decode().c_str();
        } else {
            vcc.on_click_member =
                this->on_click_func_->member_.decodeW().c_str();
            vcc.on_click_field = this->on_click_func_->field_.decodeW().c_str();
        }
    } else {
        vcc.on_click_member = nullptr;
        vcc.on_click_field = nullptr;
    }
    if (this->text_ref_) {
        if constexpr (v_index == 0) {
            vcc.text_ref_member = this->text_ref_->member_.decode().c_str();
            vcc.text_ref_field = this->text_ref_->field_.decode().c_str();
        } else {
            vcc.text_ref_member = this->text_ref_->member_.decodeW().c_str();
            vcc.text_ref_field = this->text_ref_->field_.decodeW().c_str();
        }
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
    for (const auto &o : this->option_) {
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
    return cDataT<wcfViewComponent, wcfMultiVal, 0>();
}
wcfViewComponentW ViewComponent::cDataW() const {
    return cDataT<wcfViewComponentW, wcfMultiValW, 1>();
}

Message::ViewComponent ViewComponent::toMessage() const {
    Message::ViewComponent vc;
    vc.type = static_cast<int>(this->type_);
    vc.text = this->text_;
    if (this->on_click_func_) {
        vc.on_click_member = this->on_click_func_->member_;
        vc.on_click_field = this->on_click_func_->field_;
    }
    if (this->text_ref_) {
        vc.text_ref_member = this->text_ref_->member_;
        vc.text_ref_field = this->text_ref_->field_;
    }
    vc.text_color = static_cast<int>(this->text_color_);
    vc.bg_color = static_cast<int>(this->bg_color_);
    vc.min_ = this->min_;
    vc.max_ = this->max_;
    vc.step_ = this->step_;
    vc.option_ = this->option_;
    return vc;
}
ViewComponent::ViewComponent(const Message::ViewComponent &vc)
    : ViewComponent(static_cast<ViewComponentType>(vc.type), vc.text,
                    (vc.on_click_member && vc.on_click_field
                         ? std::make_optional<FieldBase>(*vc.on_click_member,
                                                         *vc.on_click_field)
                         : std::nullopt),
                    (vc.text_ref_member && vc.text_ref_field
                         ? std::make_optional<FieldBase>(*vc.text_ref_member,
                                                         *vc.text_ref_field)
                         : std::nullopt),
                    static_cast<ViewColor>(vc.text_color),
                    static_cast<ViewColor>(vc.bg_color), vc.min_, vc.max_,
                    vc.step_, std::vector(vc.option_)) {}


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

WEBCFACE_NS_END
