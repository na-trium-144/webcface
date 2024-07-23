#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/config.h"
#endif
#include <vector>

WEBCFACE_NS_BEGIN
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

WEBCFACE_NS_END
