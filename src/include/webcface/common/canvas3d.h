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
};

} // namespace Common
} // namespace WEBCFACE_NS
