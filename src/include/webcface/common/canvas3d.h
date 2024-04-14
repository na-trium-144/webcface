#pragma once
#include "def.h"
#include "transform.h"
#include "field_base.h"
#include "view.h"
#include <vector>
#include <optional>
#include <unordered_map>

WEBCFACE_NS_BEGIN
inline namespace Common {

/*!
 * \brief Canvas3Dと2Dで共用、図形の種類を表す
 *
 */
enum class GeometryType {
    none = 0,
    line = 1,
    plane = 2,
    rect = 2,
    box = 3,
    circle = 4,
    cylinder = 5,
    sphere = 6,
    polygon = 7,
};
struct Geometry {
    GeometryType type;
    std::vector<double> properties;
    Geometry() : type(GeometryType::none), properties() {}
    Geometry(GeometryType type, const std::vector<double> &properties)
        : type(type), properties(properties) {}
    Geometry(GeometryType type, std::vector<double> &&properties)
        : type(type), properties(std::move(properties)) {}

    template <typename GeometryDerived>
    GeometryDerived as() const {
        return GeometryDerived{*this};
    }

    bool operator==(const Geometry &other) const {
        return type == other.type && properties == other.properties;
    }
    bool operator!=(const Geometry &other) const { return !(*this == other); }
};
struct Geometry3D {};
struct Geometry2D {};

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
    std::unordered_map<std::size_t, double> angles_;

    bool operator==(const Canvas3DComponentBase &rhs) const {
        return type_ == rhs.type_ && origin_ == rhs.origin_ &&
               color_ == rhs.color_ && geometry_ == rhs.geometry_ &&
               field_base_ == rhs.field_base_ && angles_ == rhs.angles_;
    }
    bool operator!=(const Canvas3DComponentBase &rhs) const {
        return !(*this == rhs);
    }
};

} // namespace Common
WEBCFACE_NS_END
