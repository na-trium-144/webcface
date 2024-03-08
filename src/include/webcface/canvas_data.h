#pragma once
#include "common/canvas2d.h"
#include "common/canvas3d.h"
#include "func.h"
#include <memory>

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
class RobotModel;

/*!
 * \brief Canvas3Dに表示する要素
 *
 */
class Canvas3DComponent : protected Common::Canvas3DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;

  public:
    Canvas3DComponent() = default;
    Canvas3DComponent(const Common::Canvas3DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas3DComponentBase(vc), data_w(data_w) {}
    explicit Canvas3DComponent(const Common::Canvas3DComponentBase &vc)
        : Common::Canvas3DComponentBase(vc), data_w() {}
    Canvas3DComponent(Canvas3DComponentType type,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas3DComponentBase(), data_w(data_w) {
        type_ = type;
    }
    explicit Canvas3DComponent(Canvas3DComponentType type)
        : Common::Canvas3DComponentBase(), data_w() {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncをFuncオブジェクトにlock
     *
     * 現状Funcをセットする要素無いのでなにもしない
     *
     */
    Canvas3DComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> & /*data_w*/,
            const std::string & /*field_id*/) {
        return *this;
    }


    /*!
     * \brief 要素の種類
     *
     */
    Canvas3DComponentType type() const { return type_; }
    /*!
     * \brief 要素の移動
     *
     */
    Transform origin() const { return origin_; }
    /*!
     * \brief 要素の移動
     *
     */
    Canvas3DComponent &origin(const Transform &origin) {
        origin_ = origin;
        return *this;
    }
    /*!
     * \brief 色
     *
     */
    ViewColor color() const { return color_; }
    /*!
     * \brief 色
     *
     */
    Canvas3DComponent &color(ViewColor color) {
        color_ = color;
        return *this;
    }
    /*!
     * \brief geometryを取得
     *
     */
    const std::optional<Geometry> &geometry() const { return geometry_; };
    /*!
     * \brief geometryをセット
     *
     */
    Canvas3DComponent &geometry(Geometry &&g) {
        geometry_.emplace(std::move(g));
        return *this;
    };
    /*!
     * \brief RobotModelを取得
     *
     */
    WEBCFACE_DLL std::optional<RobotModel> robotModel() const;
    WEBCFACE_DLL Canvas3DComponent &robotModel(const RobotModel &field);
    /*!
     * \brief RobotModelの関節をまとめて設定
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    WEBCFACE_DLL Canvas3DComponent &
    angles(const std::unordered_map<std::string, double> &angles);
    /*!
     * \brief RobotModelの関節を設定
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    WEBCFACE_DLL Canvas3DComponent &angle(const std::string &joint_name,
                                          double angle);
};

/*!
 * \brief Canvas2Dの各要素を表すクラス。
 *
 * Geometryをセットするときは CanvasCommonComponent を使うので、
 * 今のところこのクラスのオブジェクトのデータを変更する用途はない。
 *
 */
class Canvas2DComponent : protected Common::Canvas2DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;
    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

  public:
    Canvas2DComponent() = default;
    Canvas2DComponent(const Common::Canvas2DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas2DComponentBase(vc), data_w(data_w),
          on_click_func_tmp(nullptr) {}
    explicit Canvas2DComponent(const Common::Canvas2DComponentBase &vc)
        : Common::Canvas2DComponentBase(vc), data_w(),
          on_click_func_tmp(nullptr) {}
    Canvas2DComponent(Canvas2DComponentType type,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas2DComponentBase(), data_w(data_w),
          on_click_func_tmp(nullptr) {
        type_ = type;
    }
    explicit Canvas2DComponent(Canvas2DComponentType type)
        : Common::Canvas2DComponentBase(), data_w(),
          on_click_func_tmp(nullptr) {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncをFuncオブジェクトにlock
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
    Canvas2DComponent &origin(const Transform &origin) {
        origin_ = origin;
        return *this;
    }
    /*!
     * \brief 色
     *
     */
    ViewColor color() const { return color_; }
    Canvas2DComponent &color(const ViewColor &color) {
        color_ = color;
        return *this;
    }
    /*!
     * \brief 塗りつぶし色
     *
     */
    ViewColor fillColor() const { return fill_; }
    Canvas2DComponent &fillColor(const ViewColor &color) {
        fill_ = color;
        return *this;
    }
    /*!
     * \brief 線の太さ
     *
     */
    double strokeWidth() const { return stroke_width_; }
    Canvas2DComponent &strokeWidth(double s) {
        stroke_width_ = s;
        return *this;
    }
    /*!
     * \brief geometryを取得
     *
     */
    const std::optional<Geometry> &geometry() const { return geometry_; };
    /*!
     * \brief geometryをセット
     *
     */
    Canvas2DComponent &geometry(Geometry &&g) {
        geometry_.emplace(std::move(g));
        return *this;
    };
    /*!
     * \brief クリック時に実行される関数を取得
     *
     */
    WEBCFACE_DLL std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     *
     */
    WEBCFACE_DLL Canvas2DComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     *
     */
    template <typename T>
    Canvas2DComponent &onClick(const T &func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
};

/*!
 * \brief Canvas2D, Canvas3DにGeometryをaddするときに使うインタフェース
 *
 * Geometryにオプションを追加して、
 * add時にCanvas2DComponentかCanvas3DComponentにキャストする
 *
 * RobotLink用にGeometryにキャストすることもできる
 *
 */
class CanvasCommonComponent {
    Geometry geometry_common;
    std::optional<Canvas2DComponent> component_2d;
    std::optional<Canvas3DComponent> component_3d;

    void init2() {
        if (!component_2d) {
            component_2d.emplace(Canvas2DComponentType::geometry);
        }
    }
    void init3() {
        if (!component_3d) {
            component_3d.emplace(Canvas3DComponentType::geometry);
        }
    }
    void init() {
        init2();
        init3();
    }

  public:
    CanvasCommonComponent() = default;
    CanvasCommonComponent(GeometryType type, std::vector<double> &&properties)
        : geometry_common(type, std::move(properties)),
          component_2d(std::nullopt), component_3d(std::nullopt) {}

    operator Geometry &() { return geometry_common; }
    operator const Geometry &() const { return geometry_common; }
    operator Geometry &&() && { return std::move(geometry_common); }
    operator Canvas2DComponent &() {
        init2();
        component_2d->geometry(std::move(geometry_common));
        return *component_2d;
    }
    operator Canvas2DComponent &&() && {
        init2();
        component_2d->geometry(std::move(geometry_common));
        return std::move(*component_2d);
    }
    operator Canvas3DComponent &() {
        init3();
        component_3d->geometry(std::move(geometry_common));
        return *component_3d;
    }
    operator Canvas3DComponent &&() && {
        init3();
        component_3d->geometry(std::move(geometry_common));
        return std::move(*component_3d);
    }

    /*!
     * \brief クリック時に実行される関数を設定
     *
     */
    template <typename T>
    CanvasCommonComponent &onClick(const T &func) {
        init2();
        component_2d->onClick(func);
        return *this;
    }
    /*!
     * \brief 要素の移動
     *
     */
    CanvasCommonComponent &origin(const Transform &origin) {
        init();
        component_2d->origin(origin);
        component_3d->origin(origin);
        return *this;
    }
    /*!
     * \brief 色
     *
     */
    CanvasCommonComponent &color(ViewColor c) {
        init();
        component_2d->color(c);
        component_3d->color(c);
        return *this;
    }
    /*!
     * \brief 背景色 (2Dのみ)
     *
     */
    CanvasCommonComponent &fillColor(ViewColor c) {
        init2();
        component_2d->fillColor(c);
        return *this;
    }
    /*!
     * \brief 線の太さ (2Dのみ)
     *
     */
    CanvasCommonComponent &strokeWidth(double s) {
        init2();
        component_2d->strokeWidth(s);
        return *this;
    }
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
