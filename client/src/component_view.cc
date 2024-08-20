#include "webcface/component_view.h"
#include "webcface/internal/client_internal.h"
#include "webcface/field.h"
#include "webcface/encoding/encoding.h"
#include "webcface/message/message.h"
#include "webcface/internal/view_internal.h"

WEBCFACE_NS_BEGIN

ViewComponent::ViewComponent() = default;
ViewComponent::ViewComponent(ViewComponentType type, const SharedString &text,
                             std::optional<FieldBase> &&on_click_func,
                             std::optional<FieldBase> &&text_ref,
                             ViewColor text_color, ViewColor bg_color,
                             std::optional<double> min,
                             std::optional<double> max,
                             std::optional<double> step,
                             std::vector<ValAdaptor> &&option)
    : data(std::make_unique<internal::ViewComponentData>(
          internal::ViewComponentData{std::weak_ptr<internal::ClientData>(),
                                      0,
                                      type,
                                      text,
                                      std::move(on_click_func),
                                      std::move(text_ref),
                                      text_color,
                                      bg_color,
                                      min,
                                      max,
                                      step,
                                      std::move(option),
                                      nullptr,
                                      std::nullopt,
                                      std::nullopt,
                                      {},
                                      {}})) {}

ViewComponent::ViewComponent(const ViewComponent &vc,
                             const std::weak_ptr<internal::ClientData> &data_w,
                             std::unordered_map<int, int> *idx_next)
    : ViewComponent(vc) {
    checkData();
    this->data->data_w = data_w;
    if (idx_next) {
        data->idx_for_type = (*idx_next)[static_cast<int>(data->type_)]++;
    }
}
ViewComponent::ViewComponent(ViewComponentType type)
    : data(std::make_unique<internal::ViewComponentData>()) {
    this->data->type_ = type;
}
ViewComponent::ViewComponent(const ViewComponent &other)
    : data(std::make_unique<internal::ViewComponentData>(*other.data)) {}
ViewComponent &ViewComponent::operator=(const ViewComponent &other) {
    data = std::make_unique<internal::ViewComponentData>(*other.data);
    return *this;
}
ViewComponent::~ViewComponent() noexcept {}

void ViewComponent::checkData() const {
    if (!this->data) {
        throw std::runtime_error("Accessed empty ViewComponent");
    }
}

ViewComponent &
ViewComponent::lockTmp(const std::shared_ptr<internal::ClientData> &data,
                       const SharedString &view_name,
                       std::unordered_map<int, int> *idx_next) {
    checkData();
    this->data->data_w = data;
    if (idx_next) {
        this->data->idx_for_type =
            (*idx_next)[static_cast<int>(this->data->type_)]++;
    }
    if (this->data->on_click_func_tmp) {
        Func on_click{Field{this->data->data_w, data->self_member_name},
                      SharedString::fromU8String("..v" + view_name.u8String() +
                                                 "/" + id())};
        this->data->on_click_func_tmp->lockTo(on_click);
        onClick(on_click);
    }
    if (this->data->text_ref_tmp) {
        // if (text_ref_tmp->expired()) {
        Text text_ref{Field{this->data->data_w, data->self_member_name},
                      SharedString::fromU8String("..ir" + view_name.u8String() +
                                                 "/" + id())};
        this->data->text_ref_tmp->lockTo(text_ref);
        if (this->data->init_ && !text_ref.tryGet()) {
            text_ref.set(*this->data->init_);
        }
        // }
        this->data->text_ref_.emplace(static_cast<FieldBase>(text_ref));
    }
    return *this;
}

template <typename CComponent, typename CVal, std::size_t v_index>
CComponent internal::ViewComponentData::cDataT() const {
    CComponent vcc;
    vcc.type = static_cast<wcfViewComponentType>(this->type_);
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
    vcc.text_color = static_cast<wcfColor>(this->text_color_);
    vcc.bg_color = static_cast<wcfColor>(this->bg_color_);
    vcc.min = this->min_.value_or(-DBL_MAX);
    vcc.max = this->max_.value_or(DBL_MAX);
    vcc.step = this->step_.value_or(0);
    std::vector<CVal> options;
    options.reserve(this->option_.size());
    for (const auto &o : this->option_) {
        CVal val;
        val.as_int = o;
        val.as_double = o;
        val.as_str = o;
        options.push_back(val);
    }
    if constexpr (v_index == 0) {
        this->options_s = std::move(options);
        vcc.option = this->options_s.data();
    } else {
        this->options_sw = std::move(options);
        vcc.option = this->options_sw.data();
    }
    vcc.option_num = static_cast<int>(this->option_.size());
    return vcc;
}

wcfViewComponent ViewComponent::cData() const {
    checkData();
    return this->data->cDataT<wcfViewComponent, wcfMultiVal, 0>();
}
wcfViewComponentW ViewComponent::cDataW() const {
    checkData();
    return this->data->cDataT<wcfViewComponentW, wcfMultiValW, 1>();
}

message::ViewComponent ViewComponent::toMessage() const {
    checkData();
    message::ViewComponent vc;
    vc.type = static_cast<int>(this->data->type_);
    vc.text = this->data->text_;
    if (this->data->on_click_func_) {
        vc.on_click_member = this->data->on_click_func_->member_;
        vc.on_click_field = this->data->on_click_func_->field_;
    }
    if (this->data->text_ref_) {
        vc.text_ref_member = this->data->text_ref_->member_;
        vc.text_ref_field = this->data->text_ref_->field_;
    }
    vc.text_color = static_cast<int>(this->data->text_color_);
    vc.bg_color = static_cast<int>(this->data->bg_color_);
    vc.min_ = this->data->min_;
    vc.max_ = this->data->max_;
    vc.step_ = this->data->step_;
    vc.option_ = this->data->option_;
    return vc;
}
ViewComponent::ViewComponent(const message::ViewComponent &vc)
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

std::string ViewComponent::id() const {
    checkData();
    return ".." + std::to_string(static_cast<int>(type())) + "." +
           std::to_string(data->idx_for_type);
}

bool ViewComponent::operator==(const ViewComponent &other) const {
    return data && other.data && id() == other.id() &&
           data->type_ == other.data->type_ &&
           data->text_ == other.data->text_ &&
           data->on_click_func_ == other.data->on_click_func_ &&
           data->text_ref_ == other.data->text_ref_ &&
           data->text_color_ == other.data->text_color_ &&
           data->bg_color_ == other.data->bg_color_ &&
           data->min_ == other.data->min_ && data->max_ == other.data->max_ &&
           data->step_ == other.data->step_ &&
           data->option_ == other.data->option_;
}
ViewComponentType ViewComponent::type() const {
    checkData();
    return data->type_;
}
std::string ViewComponent::text() const {
    checkData();
    return data->text_.decode();
}
std::wstring ViewComponent::textW() const {
    checkData();
    return data->text_.decodeW();
}
ViewComponent &ViewComponent::text(std::string_view text) {
    checkData();
    data->text_ = SharedString::encode(text);
    return *this;
}
ViewComponent &ViewComponent::text(std::wstring_view text) {
    checkData();
    data->text_ = SharedString::encode(text);
    return *this;
}

std::optional<Func> ViewComponent::onClick() const {
    checkData();
    if (data->on_click_func_) {
        // assert(data_w.lock() != nullptr && "ClientData not set");
        return Field{data->data_w, data->on_click_func_->member_,
                     data->on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
ViewComponent &ViewComponent::onClick(const Func &func) {
    checkData();
    data->on_click_func_.emplace(static_cast<FieldBase>(func));
    return *this;
}
ViewComponent &
ViewComponent::onClick(const std::shared_ptr<AnonymousFunc> &func) {
    checkData();
    data->on_click_func_tmp = func;
    return *this;
}
ViewComponent &ViewComponent::bind(const InputRef &ref) {
    checkData();
    data->on_click_func_tmp = std::make_shared<AnonymousFunc>(
        [ref](const ValAdaptor &val) { ref.lockedField().set(val); });
    data->text_ref_tmp = ref;
    return *this;
}


std::optional<Text> ViewComponent::bind() const {
    checkData();
    if (data->text_ref_) {
        return Field{data->data_w, data->text_ref_->member_,
                     data->text_ref_->field_};
    } else {
        return std::nullopt;
    }
}

ViewComponent &
ViewComponent::onChange(const std::shared_ptr<AnonymousFunc> &func,
                        const InputRef &ref) {
    checkData();
    data->on_click_func_tmp = func;
    data->text_ref_tmp = ref;
    return *this;
}

ViewColor ViewComponent::textColor() const {
    checkData();
    return data->text_color_;
}
ViewComponent &ViewComponent::textColor(ViewColor c) {
    checkData();
    data->text_color_ = c;
    return *this;
}
ViewColor ViewComponent::bgColor() const {
    checkData();
    return data->bg_color_;
}
ViewComponent &ViewComponent::bgColor(ViewColor c) {
    checkData();
    data->bg_color_ = c;
    return *this;
}
ViewComponent &ViewComponent::init(const ValAdaptor &init) {
    checkData();
    data->init_ = init;
    return *this;
}
std::optional<double> ViewComponent::min() const {
    checkData();
    return data->min_;
}
ViewComponent &ViewComponent::min(double min) {
    checkData();
    data->min_ = min;
    data->option_.clear();
    return *this;
}
std::optional<double> ViewComponent::max() const {
    checkData();
    return data->max_;
}
ViewComponent &ViewComponent::max(double max) {
    checkData();
    data->max_ = max;
    data->option_.clear();
    return *this;
}
std::optional<double> ViewComponent::step() const {
    checkData();
    return data->step_;
}
ViewComponent &ViewComponent::step(double step) {
    checkData();
    data->step_ = step;
    return *this;
}
std::vector<ValAdaptor> ViewComponent::option() const {
    checkData();
    return data->option_;
}
ViewComponent &ViewComponent::option(std::vector<ValAdaptor> option) {
    checkData();
    data->option_ = std::move(option);
    data->min_ = data->max_ = std::nullopt;
    return *this;
}


WEBCFACE_NS_END
