#include "webcface/component_view.h"
#include "webcface/internal/client_internal.h"
#include "webcface/field.h"
#include "webcface/encoding/encoding.h"
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN

namespace internal {
struct ViewComponentData : message::ViewComponent {
    ViewComponentData() = default;

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;

    // for cData()
    mutable std::vector<wcfMultiVal> options_s;
    mutable std::vector<wcfMultiValW> options_sw;

    template <typename CComponent, typename CVal, std::size_t v_index>
    CComponent cDataT() const;
};
} // namespace internal

static inline std::string internalViewId(int type, int idx) {
    return ".." + std::to_string(type) + "." + std::to_string(idx);
}
std::string ViewComponent::id() const {
    return internalViewId(static_cast<int>(type()), idx_for_type);
}

ViewComponent::ViewComponent() = default;

/*ViewComponent::ViewComponent(ViewComponentType type, const SharedString &text,
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
*/
ViewComponent::ViewComponent(
    const std::shared_ptr<internal::ViewComponentData> &msg_data,
    const std::weak_ptr<internal::ClientData> &data_w,
    std::unordered_map<ViewComponentType, int> *idx_next)
    : msg_data(msg_data), data_w(data_w) {
    if (idx_next) {
        idx_for_type =
            (*idx_next)[static_cast<ViewComponentType>(msg_data->type)]++;
    }
}

TemporalViewComponent::TemporalViewComponent(std::nullptr_t) : msg_data() {}
TemporalViewComponent::TemporalViewComponent(ViewComponentType type)
    : msg_data(std::make_unique<internal::ViewComponentData>()) {
    this->msg_data->type = static_cast<int>(type);
}
TemporalViewComponent::TemporalViewComponent(const TemporalViewComponent &other)
    : msg_data(std::make_unique<internal::ViewComponentData>(*other.msg_data)) {
}
TemporalViewComponent &
TemporalViewComponent::operator=(const TemporalViewComponent &other) {
    msg_data = std::make_unique<internal::ViewComponentData>(*other.msg_data);
    return *this;
}
TemporalViewComponent::~TemporalViewComponent() noexcept {}

void ViewComponent::checkData() const {
    if (!this->msg_data) {
        throw std::runtime_error("Accessed empty ViewComponent");
    }
}

TemporalViewComponent &TemporalViewComponent::lockTmp(
    const std::shared_ptr<internal::ClientData> &data,
    const SharedString &view_name,
    std::unordered_map<ViewComponentType, int> *idx_next) {
    int idx_for_type = 0;
    if (idx_next) {
        idx_for_type =
            (*idx_next)[static_cast<ViewComponentType>(msg_data->type)]++;
    }
    if (msg_data->on_click_func_tmp) {
        Func on_click{Field{data, data->self_member_name},
                      SharedString::fromU8String(
                          "..v" + view_name.u8String() + "/" +
                          internalViewId(msg_data->type, idx_for_type))};
        msg_data->on_click_func_tmp->lockTo(on_click);
        onClick(on_click);
    }
    if (msg_data->text_ref_tmp) {
        // if (text_ref_tmp->expired()) {
        Text text_ref{Field{data, data->self_member_name},
                      SharedString::fromU8String(
                          "..ir" + view_name.u8String() + "/" +
                          internalViewId(msg_data->type, idx_for_type))};
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
    return *this;
}

template <typename CComponent, typename CVal, std::size_t v_index>
CComponent internal::ViewComponentData::cDataT() const {
    CComponent vcc;
    vcc.type = static_cast<wcfViewComponentType>(this->type);
    if constexpr (v_index == 0) {
        vcc.text = this->text.decode().c_str();
    } else {
        vcc.text = this->text.decodeW().c_str();
    }
    if (this->on_click_member && this->on_click_field) {
        if constexpr (v_index == 0) {
            vcc.on_click_member = this->on_click_member->decode().c_str();
            vcc.on_click_field = this->on_click_field->decode().c_str();
        } else {
            vcc.on_click_member = this->on_click_member->decodeW().c_str();
            vcc.on_click_field = this->on_click_field->decodeW().c_str();
        }
    } else {
        vcc.on_click_member = nullptr;
        vcc.on_click_field = nullptr;
    }
    if (this->text_ref_member && this->text_ref_field) {
        if constexpr (v_index == 0) {
            vcc.text_ref_member = this->text_ref_member->decode().c_str();
            vcc.text_ref_field = this->text_ref_field->decode().c_str();
        } else {
            vcc.text_ref_member = this->text_ref_member->decodeW().c_str();
            vcc.text_ref_field = this->text_ref_field->decodeW().c_str();
        }
    } else {
        vcc.text_ref_member = nullptr;
        vcc.text_ref_field = nullptr;
    }
    vcc.text_color = static_cast<wcfColor>(this->text_color);
    vcc.bg_color = static_cast<wcfColor>(this->bg_color);
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
    return msg_data->cDataT<wcfViewComponent, wcfMultiVal, 0>();
}
wcfViewComponentW ViewComponent::cDataW() const {
    checkData();
    return msg_data->cDataT<wcfViewComponentW, wcfMultiValW, 1>();
}

/*message::ViewComponent ViewComponent::toMessage() const {
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
*/

bool ViewComponent::operator==(const ViewComponent &other) const {
    return msg_data && other.msg_data && id() == other.id() &&
           msg_data->type == other.msg_data->type &&
           msg_data->text == other.msg_data->text &&
           msg_data->on_click_member == other.msg_data->on_click_member &&
           msg_data->on_click_field == other.msg_data->on_click_field &&
           msg_data->text_ref_member == other.msg_data->text_ref_member &&
           msg_data->text_ref_field == other.msg_data->text_ref_field &&
           msg_data->text_color == other.msg_data->text_color &&
           msg_data->bg_color == other.msg_data->bg_color &&
           msg_data->min_ == other.msg_data->min_ &&
           msg_data->max_ == other.msg_data->max_ &&
           msg_data->step_ == other.msg_data->step_ &&
           msg_data->option_ == other.msg_data->option_;
}
ViewComponentType ViewComponent::type() const {
    checkData();
    return static_cast<ViewComponentType>(msg_data->type);
}
std::string ViewComponent::text() const {
    checkData();
    return msg_data->text.decode();
}
std::wstring ViewComponent::textW() const {
    checkData();
    return msg_data->text.decodeW();
}
TemporalViewComponent &TemporalViewComponent::text(std::string_view text) {
    msg_data->text = SharedString::encode(text);
    return *this;
}
TemporalViewComponent &TemporalViewComponent::text(std::wstring_view text) {
    msg_data->text = SharedString::encode(text);
    return *this;
}

std::optional<Func> ViewComponent::onClick() const {
    checkData();
    if (msg_data->on_click_member && msg_data->on_click_field) {
        // assert(data_w.lock() != nullptr && "ClientData not set");
        return Field{data_w, *msg_data->on_click_member,
                     *msg_data->on_click_field};
    } else {
        return std::nullopt;
    }
}
TemporalViewComponent &TemporalViewComponent::onClick(const Func &func) {
    msg_data->on_click_member.emplace(static_cast<FieldBase>(func).member_);
    msg_data->on_click_field.emplace(static_cast<FieldBase>(func).field_);
    return *this;
}
TemporalViewComponent &
TemporalViewComponent::onClick(const std::shared_ptr<AnonymousFunc> &func) {
    msg_data->on_click_func_tmp = func;
    return *this;
}
TemporalViewComponent &TemporalViewComponent::bind(const InputRef &ref) {
    msg_data->on_click_func_tmp = std::make_shared<AnonymousFunc>(
        [ref](const ValAdaptor &val) { ref.lockedField().set(val); });
    msg_data->text_ref_tmp = ref;
    return *this;
}
std::optional<Text> ViewComponent::bind() const {
    checkData();
    if (msg_data->text_ref_member && msg_data->text_ref_field) {
        return Field{data_w, *msg_data->text_ref_member,
                     *msg_data->text_ref_field};
    } else {
        return std::nullopt;
    }
}

TemporalViewComponent &
TemporalViewComponent::onChange(const std::shared_ptr<AnonymousFunc> &func,
                                const InputRef &ref) {
    msg_data->on_click_func_tmp = func;
    msg_data->text_ref_tmp = ref;
    return *this;
}

ViewColor ViewComponent::textColor() const {
    checkData();
    return static_cast<ViewColor>(msg_data->text_color);
}
TemporalViewComponent &TemporalViewComponent::textColor(ViewColor c) {
    msg_data->text_color = static_cast<int>(c);
    return *this;
}
ViewColor ViewComponent::bgColor() const {
    checkData();
    return static_cast<ViewColor>(msg_data->bg_color);
}
TemporalViewComponent &TemporalViewComponent::bgColor(ViewColor c) {
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
TemporalViewComponent &TemporalViewComponent::min(double min) {
    msg_data->min_ = min;
    msg_data->option_.clear();
    return *this;
}
std::optional<double> ViewComponent::max() const {
    checkData();
    return msg_data->max_;
}
TemporalViewComponent &TemporalViewComponent::max(double max) {
    msg_data->max_ = max;
    msg_data->option_.clear();
    return *this;
}
std::optional<double> ViewComponent::step() const {
    checkData();
    return msg_data->step_;
}
TemporalViewComponent &TemporalViewComponent::step(double step) {
    msg_data->step_ = step;
    return *this;
}
std::vector<ValAdaptor> ViewComponent::option() const {
    checkData();
    return msg_data->option_;
}
TemporalViewComponent &
TemporalViewComponent::option(std::vector<ValAdaptor> option) {
    msg_data->option_ = std::move(option);
    msg_data->min_ = msg_data->max_ = std::nullopt;
    return *this;
}

WEBCFACE_NS_END
