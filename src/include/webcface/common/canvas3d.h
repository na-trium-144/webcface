#pragma once
#include "def.h"
#include "transform.h"
#include "field_base.h"
#include "view.h"
#include <vector>
#include <optional>
#include <unordered_map>

namespace WEBCFACE_NS {
inline namespace Common {

enum class GeometryType {
    none = 0,
    line = 1,
    plane = 2,
    box = 3,
    circle = 4,
    cylinder = 5,
    sphere = 6,
};
struct Geometry {
    GeometryType type;
    std::vector<double> properties;
    Geometry() : type(GeometryType::none), properties() {}
    Geometry(GeometryType type, const std::vector<double> &properties)
        : type(type), properties(properties) {}
};

enum class Canvas3DComponentType {
    geometry = 0,
    robot_model = 1,
    // scatter = 2,
};

struct Canvas3DComponentBase {
    Canvas3DComponentType type_;
    Transform origin_;
    ViewColor color_;
    std::optional<Geometry> geometry_;
    std::optional<FieldBase> field_base_;
    std::unordered_map<unsigned int, double> angles_;

    bool operator==(const Canvas3DComponentBase &rhs) const {
        return type_ == rhs.type_ && origin_ == rhs.origin_ &&
               color_ == rhs.color_ &&
               ((geometry_ == std::nullopt && rhs.geometry_ == std::nullopt) ||
                (geometry_ && rhs.geometry_ &&
                 geometry_->type == rhs.geometry_->type &&
                 geometry_->properties == rhs.geometry_->properties)) &&
               ((field_base_ == std::nullopt &&
                 rhs.field_base_ == std::nullopt) ||
                (field_base_ && rhs.field_base_ &&
                 field_base_->member_ == rhs.field_base_->member_ &&
                 field_base_->field_ == rhs.field_base_->field_)) &&
               angles_ == rhs.angles_;
    }
    bool operator!=(const Canvas3DComponentBase &rhs) const {
        return !(*this == rhs);
    }
};

} // namespace Common
} // namespace WEBCFACE_NS
