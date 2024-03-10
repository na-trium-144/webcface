#pragma once
#include <vector>
#include <optional>
#include <stdexcept>
#include "def.h"
#include "transform.h"
#include "view.h"
#include "canvas3d.h"
#include "field_base.h"

namespace WEBCFACE_NS {
inline namespace Common {

enum class Canvas2DComponentType {
    geometry = 0,
};

struct Canvas2DComponentBase {
    Canvas2DComponentType type_;
    Transform origin_;
    ViewColor color_, fill_;
    double stroke_width_;
    std::optional<Geometry> geometry_;
    std::optional<FieldBase> on_click_func_;

    bool operator==(const Canvas2DComponentBase &rhs) const {
        return type_ == rhs.type_ && origin_ == rhs.origin_ &&
               color_ == rhs.color_ && fill_ == rhs.fill_ &&
               stroke_width_ == rhs.stroke_width_ &&
               ((geometry_ == std::nullopt && rhs.geometry_ == std::nullopt) ||
                (geometry_ && rhs.geometry_ &&
                 geometry_->type == rhs.geometry_->type &&
                 geometry_->properties == rhs.geometry_->properties)) &&
               ((on_click_func_ == std::nullopt &&
                 rhs.on_click_func_ == std::nullopt) ||
                (on_click_func_ && rhs.on_click_func_ &&
                 on_click_func_->member_ == rhs.on_click_func_->member_ &&
                 on_click_func_->field_ == rhs.on_click_func_->field_));
    }
    bool operator!=(const Canvas2DComponentBase &rhs) const {
        return !(*this == rhs);
    }
};

struct Canvas2DDataBase {
    double width = 0, height = 0;
    std::vector<Canvas2DComponentBase> components;
    Canvas2DDataBase() = default;
    Canvas2DDataBase(double width, double height)
        : width(width), height(height), components() {}
    Canvas2DDataBase(double width, double height,
                 std::vector<Canvas2DComponentBase> &&components)
        : width(width), height(height), components(std::move(components)) {}
};

} // namespace Common
} // namespace WEBCFACE_NS
