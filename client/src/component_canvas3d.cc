#include "webcface/robot_model.h"
#include "webcface/component_canvas3d.h"
#include "webcface/internal/component_internal.h"

WEBCFACE_NS_BEGIN

static inline std::string internalCanvas3DId(int type, int idx) {
    return ".." + std::to_string(type) + "." + std::to_string(idx);
}
std::string Canvas3DComponent::id() const { return id_.decode(); }
std::wstring Canvas3DComponent::idW() const { return id_.decodeW(); }

Canvas3DComponent::Canvas3DComponent() = default;

Canvas3DComponent::Canvas3DComponent(
    const std::shared_ptr<message::Canvas3DComponentData> &msg_data,
    const std::weak_ptr<internal::ClientData> &data_w, const SharedString &id)
    : msg_data(msg_data), data_w(data_w) , id_(id){}

TemporalCanvas3DComponent::TemporalCanvas3DComponent(std::nullptr_t)
    : msg_data() {}
TemporalCanvas3DComponent::TemporalCanvas3DComponent(Canvas3DComponentType type)
    : msg_data(std::make_unique<internal::TemporalCanvas3DComponentData>()) {
    this->msg_data->type = static_cast<int>(type);
}

TemporalCanvas3DComponent::TemporalCanvas3DComponent(
    const TemporalCanvas3DComponent &other) {
    if (other.msg_data) {
        msg_data =
            std::make_unique<internal::TemporalCanvas3DComponentData>(*other.msg_data);
    }
}
TemporalCanvas3DComponent &
TemporalCanvas3DComponent::operator=(const TemporalCanvas3DComponent &other) {
    msg_data =
        std::make_unique<internal::TemporalCanvas3DComponentData>(*other.msg_data);
    return *this;
}
TemporalCanvas3DComponent::~TemporalCanvas3DComponent() noexcept {}

void Canvas3DComponent::checkData() const {
    if (!this->msg_data) {
        throw std::runtime_error("Accessed empty Canvas3DComponent");
    }
}

std::unique_ptr<internal::TemporalCanvas3DComponentData>
TemporalCanvas3DComponent::lockTmp(
    const std::shared_ptr<internal::ClientData> & /*data*/,
    const SharedString & /*view_name*/,
    std::unordered_map<Canvas3DComponentType, int> *idx_next) {
    int idx_for_type = 0;
    if (idx_next) {
        idx_for_type =
            (*idx_next)[static_cast<Canvas3DComponentType>(msg_data->type)]++;
    }
    if (msg_data->id.empty()) {
        msg_data->id = SharedString::fromU8String(
            internalCanvas3DId(msg_data->type, idx_for_type));
    }
    return std::move(msg_data);
}
bool Canvas3DComponent::operator==(const Canvas3DComponent &other) const {
    return msg_data && other.msg_data && /*id() == other.id() && */
           *msg_data == *other.msg_data;
}

TemporalCanvas3DComponent &TemporalCanvas3DComponent::id(std::string_view id) {
    msg_data->id = SharedString::encode(id);
    return *this;
}
TemporalCanvas3DComponent &TemporalCanvas3DComponent::id(std::wstring_view id) {
    msg_data->id = SharedString::encode(id);
    return *this;
}

Canvas3DComponentType Canvas3DComponent::type() const {
    checkData();
    return static_cast<Canvas3DComponentType>(msg_data->type);
}
Transform Canvas3DComponent::origin() const {
    checkData();
    return Transform(msg_data->origin_pos, msg_data->origin_rot);
}
TemporalCanvas3DComponent &
TemporalCanvas3DComponent::origin(const Transform &origin) & {
    msg_data->origin_pos = origin.pos();
    msg_data->origin_rot = origin.rot();
    return *this;
}
ViewColor Canvas3DComponent::color() const {
    checkData();
    return static_cast<ViewColor>(msg_data->color);
}
TemporalCanvas3DComponent &TemporalCanvas3DComponent::color(ViewColor color) & {
    msg_data->color = static_cast<int>(color);
    return *this;
}
std::optional<Geometry> Canvas3DComponent::geometry() const {
    checkData();
    if (!msg_data->geometry_type ||
        msg_data->geometry_type == static_cast<int>(GeometryType::none)) {
        return std::nullopt;
    } else {
        return Geometry(static_cast<GeometryType>(*msg_data->geometry_type),
                        msg_data->geometry_properties);
    }
}
TemporalCanvas3DComponent &
TemporalCanvas3DComponent::geometry(const Geometry &g) & {
    msg_data->geometry_type = static_cast<int>(g.type);
    msg_data->geometry_properties = g.properties;
    return *this;
}
std::optional<RobotModel> Canvas3DComponent::robotModel() const {
    checkData();
    if (msg_data->field_member && msg_data->field_field &&
        msg_data->type ==
            static_cast<int>(Canvas3DComponentType::robot_model)) {
        return Field{data_w, *msg_data->field_member, *msg_data->field_field};
    } else {
        return std::nullopt;
    }
}
TemporalCanvas3DComponent &
TemporalCanvas3DComponent::robotModel(const RobotModel &field) & {
    msg_data->data_w = static_cast<Field>(field).data_w;
    msg_data->field_member = static_cast<FieldBase>(field).member_;
    msg_data->field_field = static_cast<FieldBase>(field).field_;
    return *this;
}

TemporalCanvas3DComponent &TemporalCanvas3DComponent::angles(
    const std::unordered_map<std::string, double> &angles) & {
    if (msg_data->field_member && msg_data->field_field &&
        msg_data->type ==
            static_cast<int>(Canvas3DComponentType::robot_model)) {
        RobotModel rm{Field{msg_data->data_w, *msg_data->field_member,
                            *msg_data->field_field}};
        msg_data->angles.clear();
        auto model = rm.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            auto j = model[ji].joint();
            if (angles.count(j.name())) {
                msg_data->anglesAt(ji) = angles.at(j.name());
            }
        }
        return *this;
    } else {
        throw std::invalid_argument(
            "Tried to set TemporalCanvas3DComponent::angles "
            "but robotModel not defined");
    }
}
TemporalCanvas3DComponent &TemporalCanvas3DComponent::angles(
    const std::unordered_map<std::wstring, double> &angles) & {
    if (msg_data->field_member && msg_data->field_field &&
        msg_data->type ==
            static_cast<int>(Canvas3DComponentType::robot_model)) {
        RobotModel rm{Field{msg_data->data_w, *msg_data->field_member,
                            *msg_data->field_field}};
        msg_data->angles.clear();
        auto model = rm.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            auto j = model[ji].joint();
            if (angles.count(j.nameW())) {
                msg_data->anglesAt(ji) = angles.at(j.nameW());
            }
        }
        return *this;
    } else {
        throw std::invalid_argument(
            "Tried to set TemporalCanvas3DComponent::angles "
            "but robotModel not defined");
    }
}
TemporalCanvas3DComponent &
TemporalCanvas3DComponent::angle(const std::string &joint_name,
                                 double angle) & {
    if (msg_data->field_member && msg_data->field_field &&
        msg_data->type ==
            static_cast<int>(Canvas3DComponentType::robot_model)) {
        RobotModel rm{Field{msg_data->data_w, *msg_data->field_member,
                            *msg_data->field_field}};
        auto model = rm.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            auto j = model[ji].joint();
            if (joint_name == j.name()) {
                msg_data->anglesAt(ji) = angle;
            }
        }
        return *this;
    } else {
        throw std::invalid_argument(
            "Tried to set TemporalCanvas3DComponent::angles "
            "but robotModel not defined");
    }
}
TemporalCanvas3DComponent &
TemporalCanvas3DComponent::angle(const std::wstring &joint_name,
                                 double angle) & {
    if (msg_data->field_member && msg_data->field_field &&
        msg_data->type ==
            static_cast<int>(Canvas3DComponentType::robot_model)) {
        RobotModel rm{Field{msg_data->data_w, *msg_data->field_member,
                            *msg_data->field_field}};
        auto model = rm.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            auto j = model[ji].joint();
            if (joint_name == j.nameW()) {
                msg_data->anglesAt(ji) = angle;
            }
        }
        return *this;
    } else {
        throw std::invalid_argument(
            "Tried to set TemporalCanvas3DComponent::angles "
            "but robotModel not defined");
    }
}

WEBCFACE_NS_END
