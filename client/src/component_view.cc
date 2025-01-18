#define _USE_MATH_DEFINES // NOLINT
#include <cmath>
#include "webcface/component_view.h"
#include "webcface/internal/client_internal.h"
#include "webcface/field.h"
#include "webcface/common/encoding.h"
#include "webcface/internal/component_internal.h"
#include "webcface/c_wcf/def_types.h"

WEBCFACE_NS_BEGIN

ViewColor colorFromRGB(double r, double g, double b) {
    double alpha = (2 * r - g - b) / 2;
    double beta = (g - b) * 0.866;
    double h = std::atan2(beta, alpha) / M_PI * 180;
    double c2 = alpha * alpha + beta * beta;
    double i = (r + g + b) / 3;
    if (c2 < 1.0 / 9) {
        if (i < 1.0 / 3) {
            return ViewColor::black;
        } else if (i < 2.0 / 3) {
            return ViewColor::gray;
        } else {
            return ViewColor::white;
        }
    } else {
        if (h > 330 || h < 15) {
            return ViewColor::red;
        } else if (h < 35) {
            return ViewColor::orange;
        } else if (h < 60) {
            return ViewColor::yellow;
        } else if (h < 150) {
            return ViewColor::green;
        } else if (h < 180) {
            return ViewColor::teal;
        } else if (h < 195) {
            return ViewColor::cyan;
        } else if (h < 225) {
            return ViewColor::blue;
        } else if (h < 250) {
            return ViewColor::indigo;
        } else if (h < 295) {
            return ViewColor::purple;
        } else {
            return ViewColor::pink;
        }
    }
}

/// \private
static inline std::string internalViewId(int type, int idx) {
    return ".." + std::to_string(type) + "." + std::to_string(idx);
}
std::string ViewComponent::id() const { return id_.decode(); }
std::wstring ViewComponent::idW() const { return id_.decodeW(); }

ViewComponent::ViewComponent() = default;
ViewComponent::ViewComponent(const ViewComponent &other)
    : msg_data(other.msg_data), data_w(other.data_w), id_(other.id_) {}
ViewComponent &ViewComponent::operator=(const ViewComponent &other) {
    if (this != &other) {
        msg_data = other.msg_data;
        data_w = other.data_w;
        id_ = other.id_;
    }
    return *this;
}
ViewComponent::ViewComponent(ViewComponent &&) noexcept = default;
ViewComponent &ViewComponent::operator=(ViewComponent &&) noexcept = default;
ViewComponent::~ViewComponent() noexcept = default;

ViewComponent::ViewComponent(
    const std::shared_ptr<const message::ViewComponentData> &msg_data,
    const std::weak_ptr<internal::ClientData> &data_w, const SharedString &id)
    : msg_data(msg_data), data_w(data_w), id_(id) {}

TemporalViewComponent::TemporalViewComponent(std::nullptr_t) : msg_data() {}
TemporalViewComponent::TemporalViewComponent(ViewComponentType type)
    : msg_data(std::make_unique<internal::TemporalViewComponentData>()) {
    this->msg_data->type = static_cast<int>(type);
}
TemporalViewComponent::TemporalViewComponent(const TemporalViewComponent &other)
    : msg_data(other.msg_data
                   ? std::make_unique<internal::TemporalViewComponentData>(
                         *other.msg_data)
                   : nullptr) {}
TemporalViewComponent &
TemporalViewComponent::operator=(const TemporalViewComponent &other) {
    if (this != &other && other.msg_data) {
        msg_data = std::make_unique<internal::TemporalViewComponentData>(
            *other.msg_data);
    }
    return *this;
}
TemporalViewComponent::TemporalViewComponent(
    TemporalViewComponent &&other) noexcept = default;
TemporalViewComponent &TemporalViewComponent::operator=(
    TemporalViewComponent &&other) noexcept = default;
TemporalViewComponent::~TemporalViewComponent() noexcept = default;

void ViewComponent::checkData() const {
    if (!this->msg_data) {
        throw std::runtime_error("Accessed empty ViewComponent");
    }
}

std::unique_ptr<internal::TemporalViewComponentData>
TemporalViewComponent::lockTmp(
    const std::shared_ptr<internal::ClientData> &data,
    const SharedString &view_name,
    std::unordered_map<ViewComponentType, int> *idx_next) {
    int idx_for_type = 0;
    if (idx_next) {
        idx_for_type =
            (*idx_next)[static_cast<ViewComponentType>(msg_data->type)]++;
    }
    if (msg_data->id.empty()) {
        msg_data->id = SharedString::fromU8String(
            internalViewId(msg_data->type, idx_for_type));
    }
    if (msg_data->on_click_func_tmp && !*msg_data->on_click_func_tmp) {
        msg_data->on_click_func_tmp.reset();
    }
    if (msg_data->on_change_func_tmp && !*msg_data->on_change_func_tmp) {
        msg_data->on_change_func_tmp.reset();
    }
    if (msg_data->on_click_func_tmp || msg_data->on_change_func_tmp) {
        Func on_click{Field{data, data->self_member_name},
                      SharedString::fromU8String("..v" + view_name.u8String() +
                                                 "/" +
                                                 msg_data->id.u8String())};
        if (msg_data->on_click_func_tmp && !msg_data->on_change_func_tmp) {
            on_click.set(std::move(*msg_data->on_click_func_tmp));
        } else if (msg_data->on_change_func_tmp &&
                   !msg_data->on_click_func_tmp) {
            on_click.set(std::move(*msg_data->on_change_func_tmp));
        } else {
            throw std::runtime_error("Both onClick and onChange are set");
        }
        onClick(on_click);
    }
    if (msg_data->text_ref_tmp) {
        // if (text_ref_tmp->expired()) {
        Variant text_ref{Field{data, data->self_member_name},
                         SharedString::fromU8String("..ir" +
                                                    view_name.u8String() + "/" +
                                                    msg_data->id.u8String())};
        msg_data->text_ref_tmp->lockTo(text_ref);
        if (msg_data->init_ && !text_ref.tryGet()) {
            text_ref.set(*msg_data->init_);
        }
        // }
        msg_data->text_ref_member.emplace(
            static_cast<FieldBase>(text_ref).member_);
        msg_data->text_ref_field.emplace(
            static_cast<FieldBase>(text_ref).field_);
    }
    return std::move(msg_data);
}

template <typename CComponent, typename CVal, std::size_t v_index>
CComponent ViewComponent::cDataT() const {
    CComponent vcc;
    vcc.type = static_cast<wcfViewComponentType>(msg_data->type);
    if constexpr (v_index == 0) {
        vcc.text = msg_data->text.decode().c_str();
    } else {
        vcc.text = msg_data->text.decodeW().c_str();
    }
    if (msg_data->on_click_member && msg_data->on_click_field) {
        if constexpr (v_index == 0) {
            vcc.on_click_member = msg_data->on_click_member->decode().c_str();
            vcc.on_click_field = msg_data->on_click_field->decode().c_str();
        } else {
            vcc.on_click_member = msg_data->on_click_member->decodeW().c_str();
            vcc.on_click_field = msg_data->on_click_field->decodeW().c_str();
        }
    } else {
        vcc.on_click_member = nullptr;
        vcc.on_click_field = nullptr;
    }
    if (msg_data->text_ref_member && msg_data->text_ref_field) {
        if constexpr (v_index == 0) {
            vcc.text_ref_member = msg_data->text_ref_member->decode().c_str();
            vcc.text_ref_field = msg_data->text_ref_field->decode().c_str();
        } else {
            vcc.text_ref_member = msg_data->text_ref_member->decodeW().c_str();
            vcc.text_ref_field = msg_data->text_ref_field->decodeW().c_str();
        }
    } else {
        vcc.text_ref_member = nullptr;
        vcc.text_ref_field = nullptr;
    }
    vcc.text_color = static_cast<wcfColor>(msg_data->text_color);
    vcc.bg_color = static_cast<wcfColor>(msg_data->bg_color);
    vcc.min = msg_data->min_.value_or(-DBL_MAX);
    vcc.max = msg_data->max_.value_or(DBL_MAX);
    vcc.step = msg_data->step_.value_or(0);
    vcc.option = nullptr;
    if constexpr (v_index == 0) {
        if (this->options_s) {
            vcc.option = this->options_s.get();
        }
    } else {
        if (this->options_sw) {
            vcc.option = this->options_sw.get();
        }
    }
    if (!vcc.option) {
        auto options = std::make_unique<CVal[]>(msg_data->option_.size());
        for (std::size_t i = 0; i < msg_data->option_.size(); i++) {
            CVal val;
            val.as_int = msg_data->option_[i];
            val.as_double = msg_data->option_[i];
            val.as_str = msg_data->option_[i];
            options[i] = val;
        }
        if constexpr (v_index == 0) {
            this->options_s = std::move(options);
            vcc.option = this->options_s.get();
        } else {
            this->options_sw = std::move(options);
            vcc.option = this->options_sw.get();
        }
    }
    vcc.option_num = static_cast<int>(msg_data->option_.size());
    return vcc;
}

wcfViewComponent ViewComponent::cData() const {
    checkData();
    return cDataT<wcfViewComponent, wcfMultiVal, 0>();
}
wcfViewComponentW ViewComponent::cDataW() const {
    checkData();
    return cDataT<wcfViewComponentW, wcfMultiValW, 1>();
}


bool ViewComponent::operator==(const ViewComponent &other) const {
    return msg_data && other.msg_data && id() == other.id() &&
           *msg_data == *other.msg_data;
}

ViewComponentType ViewComponent::type() const {
    checkData();
    return static_cast<ViewComponentType>(msg_data->type);
}
TemporalViewComponent &TemporalViewComponent::id(std::string_view id) {
    msg_data->id = SharedString::encode(id);
    return *this;
}
TemporalViewComponent &TemporalViewComponent::id(std::wstring_view id) {
    msg_data->id = SharedString::encode(id);
    return *this;
}
std::string ViewComponent::text() const {
    checkData();
    return msg_data->text.decode();
}
std::wstring ViewComponent::textW() const {
    checkData();
    return msg_data->text.decodeW();
}
TemporalViewComponent &TemporalViewComponent::text(std::string_view text) & {
    msg_data->text = SharedString::encode(text);
    return *this;
}
TemporalViewComponent &TemporalViewComponent::text(std::wstring_view text) & {
    msg_data->text = SharedString::encode(text);
    return *this;
}
template <typename T, bool>
std::optional<T> ViewComponent::onChange() const {
    return onClick();
}
template WEBCFACE_DLL std::optional<Func>
ViewComponent::onChange<Func, true>() const;
template <typename T, bool>
std::optional<T> ViewComponent::onClick() const {
    checkData();
    if (msg_data->on_click_member && msg_data->on_click_field) {
        // assert(data_w.lock() != nullptr && "ClientData not set");
        return Field{data_w, *msg_data->on_click_member,
                     *msg_data->on_click_field};
    } else {
        return std::nullopt;
    }
}
template WEBCFACE_DLL std::optional<Func>
ViewComponent::onClick<Func, true>() const;
TemporalViewComponent &TemporalViewComponent::onClick(const Func &func) & {
    msg_data->on_click_member.emplace(static_cast<FieldBase>(func).member_);
    msg_data->on_click_field.emplace(static_cast<FieldBase>(func).field_);
    return *this;
}
TemporalViewComponent &
TemporalViewComponent::onClick(const FuncListener &func) & {
    msg_data->on_click_member.emplace(static_cast<FieldBase>(func).member_);
    msg_data->on_click_field.emplace(static_cast<FieldBase>(func).field_);
    return *this;
}
TemporalViewComponent &TemporalViewComponent::onClick(
    const std::shared_ptr<std::function<void WEBCFACE_CALL_FP()>> &func) {
    msg_data->on_click_func_tmp = func;
    return *this;
}
TemporalViewComponent &TemporalViewComponent::bind(const InputRef &ref) & {
    msg_data->on_change_func_tmp =
        std::make_shared<std::function<void(ValAdaptor)>>(
            [ref](const ValAdaptor &val) { ref.lockedField().set(val); });
    msg_data->text_ref_tmp = ref;
    return *this;
}
template <typename T, bool>
std::optional<T> ViewComponent::bind() const {
    checkData();
    if (msg_data->text_ref_member && msg_data->text_ref_field) {
        return Field{data_w, *msg_data->text_ref_member,
                     *msg_data->text_ref_field};
    } else {
        return std::nullopt;
    }
}
template WEBCFACE_DLL std::optional<Variant>
ViewComponent::bind<Variant, true>() const;

TemporalViewComponent &TemporalViewComponent::onChange(
    const std::shared_ptr<std::function<void WEBCFACE_CALL_FP(ValAdaptor)>>
        &func,
    const InputRef &ref) {
    msg_data->on_change_func_tmp = func;
    msg_data->text_ref_tmp = ref;
    return *this;
}

ViewColor ViewComponent::textColor() const {
    checkData();
    return static_cast<ViewColor>(msg_data->text_color);
}
TemporalViewComponent &TemporalViewComponent::textColor(ViewColor c) & {
    msg_data->text_color = static_cast<int>(c);
    return *this;
}
ViewColor ViewComponent::bgColor() const {
    checkData();
    return static_cast<ViewColor>(msg_data->bg_color);
}
TemporalViewComponent &TemporalViewComponent::bgColor(ViewColor c) & {
    msg_data->bg_color = static_cast<int>(c);
    return *this;
}
TemporalViewComponent &TemporalViewComponent::init(const ValAdaptor &init) {
    msg_data->init_ = init;
    return *this;
}
std::optional<double> ViewComponent::min() const {
    checkData();
    return msg_data->min_;
}
TemporalViewComponent &TemporalViewComponent::min(double min) & {
    msg_data->min_ = min;
    msg_data->option_.clear();
    return *this;
}
std::optional<double> ViewComponent::max() const {
    checkData();
    return msg_data->max_;
}
TemporalViewComponent &TemporalViewComponent::max(double max) & {
    msg_data->max_ = max;
    msg_data->option_.clear();
    return *this;
}
std::optional<double> ViewComponent::step() const {
    checkData();
    return msg_data->step_;
}
TemporalViewComponent &TemporalViewComponent::step(double step) & {
    msg_data->step_ = step;
    return *this;
}
std::vector<ValAdaptor> ViewComponent::option() const {
    checkData();
    return msg_data->option_;
}
TemporalViewComponent &
TemporalViewComponent::option(std::vector<ValAdaptor> option) & {
    msg_data->option_ = std::move(option);
    msg_data->min_ = msg_data->max_ = std::nullopt;
    return *this;
}

int ViewComponent::width() const {
    checkData();
    return msg_data->width;
}
TemporalViewComponent &TemporalViewComponent::width(int width) & {
    msg_data->width = width;
    return *this;
}
int ViewComponent::height() const {
    checkData();
    return msg_data->height;
}
TemporalViewComponent &TemporalViewComponent::height(int height) & {
    msg_data->height = height;
    return *this;
}

WEBCFACE_NS_END
