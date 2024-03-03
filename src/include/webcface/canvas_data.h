#pragma once
#include "common/canvas2d.h"
#include "common/canvas3d.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}

/*!
 * \brief Canvas3Dに表示する要素
 *
 */
class Canvas3DComponent : public Common::Canvas3DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;
    Geometry *common_geometry_tmp;

  public:
    Canvas3DComponent() = default;
    Canvas3DComponent(const Common::Canvas3DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas3DComponentBase(vc), data_w(data_w),
          common_geometry_tmp(nullptr) {}
    explicit Canvas3DComponent(Geometry *common_geometry_tmp)
        : Common::Canvas3DComponentBase(), data_w(),
          common_geometry_tmp(common_geometry_tmp) {
        type_ = Canvas3DComponentType::geometry;
    }
};

/*!
 * \brief Canvas2Dの各要素を表すクラス。
 *
 * Geometryをセットするときは CanvasCommonComponent を使うので、
 * 今のところこのクラスのオブジェクトのデータを変更する用途はない。
 *
 */
class Canvas2DComponent : public Common::Canvas2DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;
    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    Geometry *common_geometry_tmp;

  public:
    Canvas2DComponent() = default;
    Canvas2DComponent(const Common::Canvas2DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas2DComponentBase(vc), data_w(data_w),
          on_click_func_tmp(nullptr), common_geometry_tmp(nullptr) {}
    explicit Canvas2DComponent(Geometry *common_geometry_tmp)
        : Common::Canvas2DComponentBase(), data_w(), on_click_func_tmp(nullptr),
          common_geometry_tmp(common_geometry_tmp) {
        type_ = Canvas2DComponentType::geometry;
    }

    /*!
     * \brief AnonymousFuncをFuncオブジェクトにlock
     * & geometry_tmpをBaseのgeometryにセット
     *
     */
    WEBCFACE_DLL Canvas2DComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
            const std::string &field_id);

    /*!
     * \brief 要素の種類
     *
     */
    Canvas2DComponentType type() const { return type_; }
    /*!
     * \brief 要素の移動
     *
     */
    Transform origin() const { return origin_; }
    /*!
     * \brief 色
     *
     */
    ViewColor color() const { return color_; }
    /*!
     * \brief 塗りつぶし色
     *
     */
    ViewColor fillColor() const { return fill_; }
    /*!
     * \brief 線の太さ
     *
     */
    double strokeWidth() const { return stroke_width_; }
    /*!
     * \brief geometryを取得
     *
     */
    const std::optional<Geometry> &geometry() const { return geometry_; };
    /*!
     * \brief クリック時に実行される関数を取得
     *
     */
    WEBCFACE_DLL std::optional<Func> onClick() const;
};

/*!
 * \brief Canvas2D, Canvas3DにGeometryをaddするときに使うインタフェース
 *
 * 各種Geometry型の共通クラス。
 *
 */
class CanvasCommonComponent : Canvas2DComponent, Canvas3DComponent {
    Geometry geometry_common;

  public:
    friend class Canvas2D;
    friend class Canvas3D;
    CanvasCommonComponent() = default;
    CanvasCommonComponent(GeometryType type, std::vector<double> &&properties)
        : Canvas2DComponent(&geometry_common),
          Canvas3DComponent(&geometry_common),
          geometry_common(type, std::move(properties)) {}
};

inline namespace Geometries {
struct Line {
    const Geometry &base;
    explicit Line(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point begin() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    Point end() const {
        return Point{base.properties[3], base.properties[4],
                     base.properties[5]};
    }
};
inline CanvasCommonComponent line(const Point &begin, const Point &end) {
    return CanvasCommonComponent(
        GeometryType::line, {begin.pos()[0], begin.pos()[1], begin.pos()[2],
                             end.pos()[0], end.pos()[1], end.pos()[2]});
}
struct Polygon {
    const Geometry &base;
    explicit Polygon(const Geometry &rg) : base(rg) {
        if (base.properties.size() % 3 != 0 || size() == 0) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    std::size_t size() const { return base.properties.size() / 3; }
    Point at(std::size_t i) const {
        if (i >= size()) {
            throw std::out_of_range("Polygon::at(" + std::to_string(i) +
                                    "), size = " + std::to_string(size()));
        }
        return Point{base.properties[i * 3 + 0], base.properties[i * 3 + 1],
                     base.properties[i * 3 + 2]};
    }
    Point operator[](std::size_t i) const { return at(i); }
};
inline CanvasCommonComponent polygon(const std::vector<Point> &points) {
    std::vector<double> properties;
    properties.reserve(points.size() * 3);
    for (const auto &p : points) {
        properties.push_back(p.pos(0));
        properties.push_back(p.pos(1));
        properties.push_back(p.pos(2));
    }
    return CanvasCommonComponent(GeometryType::polygon, std::move(properties));
}
struct Plane {
    const Geometry &base;
    explicit Plane(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 8) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{base.properties[0], base.properties[1],
                         base.properties[2], base.properties[3],
                         base.properties[4], base.properties[5]};
    }
    double width() const { return base.properties[6]; }
    double height() const { return base.properties[7]; }
    /*!
     * todo: 3次元のplaneの場合正しくない
     *
     */
    Point vertex1() const {
        return {base.properties[0] - width() / 2,
                base.properties[1] - height() / 2, base.properties[2]};
    }
    Point vertex2() const {
        return {base.properties[0] + width() / 2,
                base.properties[1] + height() / 2, base.properties[2]};
    }
};
inline CanvasCommonComponent plane(const Transform &origin, double width,
                                   double height) {
    return CanvasCommonComponent(
        GeometryType::plane,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], origin.rot()[0],
         origin.rot()[1], origin.rot()[2], width, height});
}
using Rect = Plane;
inline CanvasCommonComponent rect(const Point &origin, double width,
                                  double height) {
    return CanvasCommonComponent{GeometryType::plane,
                                 {origin.pos()[0], origin.pos()[1],
                                  origin.pos()[2], 0, 0, 0, width, height}};
}
inline CanvasCommonComponent rect(const Point &p1, const Point &p2) {
    Transform origin = identity();
    for (int i = 0; i < 2; i++) {
        origin.pos(i) = (p1.pos(i) + p2.pos(i)) / 2;
    }
    double width = std::abs(p1.pos(0) - p2.pos(0));
    double height = std::abs(p1.pos(0) - p2.pos(0));
    return CanvasCommonComponent{GeometryType::plane,
                                 {origin.pos(0), origin.pos(1), origin.pos(2),
                                  origin.rot(0), origin.rot(1), origin.rot(2),
                                  width, height}};
}

struct Box {
    const Geometry &base;
    explicit Box(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point vertex1() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    Point vertex2() const {
        return Point{base.properties[3], base.properties[4],
                     base.properties[5]};
    }
};
inline CanvasCommonComponent box(const Point &vertex1, const Point &vertex2) {
    return CanvasCommonComponent{GeometryType::box,
                                 {vertex1.pos()[0], vertex1.pos()[1],
                                  vertex1.pos()[2], vertex2.pos()[0],
                                  vertex2.pos()[1], vertex2.pos()[2]}};
}

struct Circle {
    const Geometry &base;
    explicit Circle(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 7) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{base.properties[0], base.properties[1],
                         base.properties[2], base.properties[3],
                         base.properties[4], base.properties[5]};
    }
    double radius() const { return base.properties[6]; }
};
inline CanvasCommonComponent circle(const Transform &origin, double radius) {
    return CanvasCommonComponent{GeometryType::circle,
                                 {origin.pos()[0], origin.pos()[1],
                                  origin.pos()[2], origin.rot()[0],
                                  origin.rot()[1], origin.rot()[2], radius}};
}
inline CanvasCommonComponent circle(const Point &origin, double radius) {
    return CanvasCommonComponent{
        GeometryType::circle,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], 0, 0, 0, radius}};
}

struct Cylinder {
    const Geometry &base;
    explicit Cylinder(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 8) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{base.properties[0], base.properties[1],
                         base.properties[2], base.properties[3],
                         base.properties[4], base.properties[5]};
    }
    double radius() const { return base.properties[6]; }
    double length() const { return base.properties[7]; }
};
inline CanvasCommonComponent cylinder(const Transform &origin, double radius,
                                      double length) {
    return CanvasCommonComponent{
        GeometryType::cylinder,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], origin.rot()[0],
         origin.rot()[1], origin.rot()[2], radius, length}};
}

struct Sphere {
    const Geometry &base;
    explicit Sphere(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 4) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point origin() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    double radius() const { return base.properties[3]; }
};
inline CanvasCommonComponent sphere(const Point &origin, double radius) {
    return CanvasCommonComponent{
        GeometryType::sphere,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], radius}};
}


} // namespace Geometries
} // namespace WEBCFACE_NS
