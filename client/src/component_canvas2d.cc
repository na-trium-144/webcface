#include "webcface/component_canvas2d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"

WEBCFACE_NS_BEGIN

static inline std::string internalCanvas2DId(int type, int idx) {
    return ".." + std::to_string(type) + "." + std::to_string(idx);
}
std::string Canvas2DComponent::id() const { return id_.decode(); }
std::wstring Canvas2DComponent::idW() const { return id_.decodeW(); }

Canvas2DComponent::Canvas2DComponent() = default;
Canvas2DComponent::Canvas2DComponent(
    const std::shared_ptr<message::Canvas2DComponentData> &msg_data,
    const std::weak_ptr<internal::ClientData> &data_w, const SharedString &id)
    : msg_data(msg_data), data_w(data_w), id_(id) {}

TemporalCanvas2DComponent::TemporalCanvas2DComponent(std::nullptr_t)
    : msg_data() {}
TemporalCanvas2DComponent::TemporalCanvas2DComponent(Canvas2DComponentType type)
    : msg_data(std::make_unique<internal::TemporalCanvas2DComponentData>()) {
    this->msg_data->type = static_cast<int>(type);
}
TemporalCanvas2DComponent::TemporalCanvas2DComponent(
    const TemporalCanvas2DComponent &other) {
    if (other.msg_data) {
        msg_data = std::make_unique<internal::TemporalCanvas2DComponentData>(
            *other.msg_data);
    }
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::operator=(const TemporalCanvas2DComponent &other) {
    msg_data = std::make_unique<internal::TemporalCanvas2DComponentData>(
        *other.msg_data);
    return *this;
}
TemporalCanvas2DComponent::TemporalCanvas2DComponent(
    TemporalCanvas2DComponent &&other) noexcept
    : msg_data(std::move(other.msg_data)) {}
TemporalCanvas2DComponent &TemporalCanvas2DComponent::operator=(
    TemporalCanvas2DComponent &&other) noexcept {
    msg_data = std::move(other.msg_data);
    return *this;
}
TemporalCanvas2DComponent::~TemporalCanvas2DComponent() noexcept {}

void Canvas2DComponent::checkData() const {
    if (!this->msg_data) {
        throw std::runtime_error("Accessed empty Canvas2DComponent");
    }
}

std::unique_ptr<internal::TemporalCanvas2DComponentData>
TemporalCanvas2DComponent::lockTmp(
    const std::shared_ptr<internal::ClientData> &data,
    const SharedString &view_name,
    std::unordered_map<Canvas2DComponentType, int> *idx_next) {
    int idx_for_type = 0;
    if (idx_next) {
        idx_for_type =
            (*idx_next)[static_cast<Canvas2DComponentType>(msg_data->type)]++;
    }
    if (msg_data->id.empty()) {
        msg_data->id = SharedString::fromU8String(
            internalCanvas2DId(msg_data->type, idx_for_type));
    }
    if (msg_data->on_click_func_tmp && *msg_data->on_click_func_tmp) {
        Func on_click{Field{data, data->self_member_name},
                      SharedString::fromU8String("..c2" + view_name.u8String() +
                                                 "/" +
                                                 msg_data->id.u8String())};
        on_click.set(std::move(*msg_data->on_click_func_tmp));
        this->onClick(on_click);
    }
    return std::move(msg_data);
}


bool Canvas2DComponent::operator==(const Canvas2DComponent &other) const {
    return msg_data && other.msg_data && id() == other.id() &&
           *msg_data == *other.msg_data;
}

TemporalCanvas2DComponent &TemporalCanvas2DComponent::id(std::string_view id) {
    msg_data->id = SharedString::encode(id);
    return *this;
}
TemporalCanvas2DComponent &TemporalCanvas2DComponent::id(std::wstring_view id) {
    msg_data->id = SharedString::encode(id);
    return *this;
}

Canvas2DComponentType Canvas2DComponent::type() const {
    checkData();
    return static_cast<Canvas2DComponentType>(msg_data->type);
}
Transform Canvas2DComponent::origin() const {
    checkData();
    return Transform(msg_data->origin_pos, msg_data->origin_rot);
}

TemporalCanvas2DComponent &
TemporalCanvas2DComponent::origin(const Transform &origin) & {
    msg_data->origin_pos = {origin.pos(0), origin.pos(1)};
    msg_data->origin_rot = origin.rotEuler()[0];
    return *this;
}
ViewColor Canvas2DComponent::color() const {
    checkData();
    return static_cast<ViewColor>(msg_data->color);
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::color(const ViewColor &color) & {
    msg_data->color = static_cast<int>(color);
    return *this;
}
ViewColor Canvas2DComponent::fillColor() const {
    checkData();
    return static_cast<ViewColor>(msg_data->fill);
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::fillColor(const ViewColor &color) & {
    msg_data->fill = static_cast<int>(color);
    return *this;
}
double Canvas2DComponent::strokeWidth() const {
    checkData();
    return msg_data->stroke_width;
}
TemporalCanvas2DComponent &TemporalCanvas2DComponent::strokeWidth(double s) & {
    msg_data->stroke_width = s;
    return *this;
}
std::string Canvas2DComponent::text() const {
    checkData();
    return msg_data->text.decode();
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::text(std::string_view text) & {
    msg_data->text = SharedString::encode(text);
    return *this;
}
std::wstring Canvas2DComponent::textW() const {
    checkData();
    return msg_data->text.decodeW();
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::text(std::wstring_view text) & {
    msg_data->text = SharedString::encode(text);
    return *this;
}
std::optional<Geometry> Canvas2DComponent::geometry() const {
    checkData();
    if (!msg_data->geometry_type ||
        *msg_data->geometry_type == static_cast<int>(GeometryType::none)) {
        return std::nullopt;
    } else {
        return Geometry(static_cast<GeometryType>(*msg_data->geometry_type),
                        msg_data->properties);
    }
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::geometry(const Geometry &g) & {
    msg_data->geometry_type = static_cast<int>(g.type);
    msg_data->properties = g.properties;
    return *this;
}

template <typename T, bool>
std::optional<T> Canvas2DComponent::onClick() const {
    checkData();
    if (msg_data->on_click_member && msg_data->on_click_field) {
        return Field{data_w, *msg_data->on_click_member,
                     *msg_data->on_click_field};
    } else {
        return std::nullopt;
    }
}
template WEBCFACE_DLL std::optional<Func>
Canvas2DComponent::onClick<Func, true>() const;
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::onClick(const Func &func) & {
    msg_data->on_click_member = static_cast<FieldBase>(func).member_;
    msg_data->on_click_field = static_cast<FieldBase>(func).field_;
    return *this;
}
TemporalCanvas2DComponent &
TemporalCanvas2DComponent::onClick(const FuncListener &func) & {
    msg_data->on_click_member = static_cast<FieldBase>(func).member_;
    msg_data->on_click_field = static_cast<FieldBase>(func).field_;
    return *this;
}
TemporalCanvas2DComponent &TemporalCanvas2DComponent::onClick(
    const std::shared_ptr<std::function<void WEBCFACE_CALL_FP()>> &func) {
    msg_data->on_click_func_tmp = func;
    return *this;
}


WEBCFACE_NS_END
