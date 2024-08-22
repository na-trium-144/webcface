#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <vector>
#include <type_traits>

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

    /*!
     * \brief 各種Geometry型に変換
     *
     * as<Line>(), as<Plane>()
     * などとしてそれぞれのgeometry型のプロパティを取得する。
     *
     */
    template <
        typename GeometryDerived,
        std::enable_if_t<std::is_constructible_v<GeometryDerived, Geometry &>,
                         std::nullptr_t> = nullptr>
    GeometryDerived as() const {
        return GeometryDerived{*this};
    }

    bool operator==(const Geometry &other) const {
        return type == other.type && properties == other.properties;
    }
    bool operator!=(const Geometry &other) const { return !(*this == other); }
};

WEBCFACE_NS_END
