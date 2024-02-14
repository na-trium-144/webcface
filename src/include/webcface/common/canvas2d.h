#pragma once
#include <vector>
#include <optional>
#include <stdexcept>
#include "def.h"
#include "transform.h"
#include "view.h"
#include "canvas3d.h"

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

    bool operator==(const Canvas2DComponentBase &rhs) const {
        return type_ == rhs.type_ && origin_ == rhs.origin_ &&
               color_ == rhs.color_ && fill_ == rhs.fill_ &&
               stroke_width_ == rhs.stroke_width_ &&
               ((geometry_ == std::nullopt && rhs.geometry_ == std::nullopt) ||
                (geometry_ && rhs.geometry_ &&
                 geometry_->type == rhs.geometry_->type &&
                 geometry_->properties == rhs.geometry_->properties));
    }
    bool operator!=(const Canvas2DComponentBase &rhs) const {
        return !(*this == rhs);
    }
};

struct Canvas2DData {
    double width = 0, height = 0;
    std::vector<Canvas2DComponentBase> components;
    Canvas2DData() = default;
    Canvas2DData(double width, double height,
                 const std::vector<Canvas2DComponentBase> &components)
        : width(width), height(height), components(components) {}
    void checkSize() const {
        if (width <= 0 && height <= 0) {
            throw std::invalid_argument("Canvas2D size is invalid (" +
                                        std::to_string(width) + ", " +
                                        std::to_string(height) + ")");
        }
    }
};

} // namespace Common
} // namespace WEBCFACE_NS
