#pragma once
#include "component_view.h"
#include "component_canvas2d.h"
#include "component_canvas3d.h"
#include "webcface/exception.h"

WEBCFACE_NS_BEGIN

namespace internal {
class ViewBuf;
}

/*!
 * \brief Canvas2D, Canvas3D (, View) に要素をaddするときに使うインタフェース
 *
 * add時に各種Componentにキャストする
 *
 * 各オプションの詳細な説明は ViewComponent, Canvas2DComponent,
 * Canvas3DComponent を参照
 *
 */
template <bool V, bool C2, bool C3>
struct TemporalComponent {
    TemporalViewComponent component_v;
    TemporalCanvas2DComponent component_2d;
    TemporalCanvas3DComponent component_3d;

    TemporalComponent() = default;
    template <typename VT, typename C2T, typename C3T>
    TemporalComponent(VT v_type, C2T c2_type, C3T c3_type)
        : component_v(v_type), component_2d(c2_type), component_3d(c3_type) {
        // V, C2, C3 がtrue <=> VT, C2T, C3T にnullptrでない値を渡して初期化する
        // V, C2, C3 がfalse <=> VT, C2T, C3T にnullptrを渡す
        static_assert(V == !std::is_same_v<VT, std::nullptr_t>);
        static_assert(C2 == !std::is_same_v<C2T, std::nullptr_t>);
        static_assert(C3 == !std::is_same_v<C3T, std::nullptr_t>);
    }

    friend class Func;
    friend class Canvas2D;
    friend class Canvas3D;
    friend class View;
    friend class internal::ViewBuf;

    /*!
     * \brief idを設定
     * \since ver2.5
     * 
     * ver2.10〜 StringInitializer 型で置き換え
     */
    TemporalComponent &id(const StringInitializer &id) & {
        if constexpr (V) {
            component_v.id(id);
        }
        if constexpr (C2) {
            component_2d.id(id);
        }
        if constexpr (C3) {
            component_3d.id(id);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&id(const StringInitializer &id) && {
        this->id(id);
        return std::move(*this);
    }

    /*!
     * \brief クリック時に実行される関数を設定 (Viewまたは2D, Funcオブジェクト)
     * \since ver2.5
     *
     * 引数については ViewComponent::onClick(), Canvas2DComponent::onClick()
     * を参照
     *
     */
    TemporalComponent &onClick(const Func &func) & {
        static_assert(V || C2,
                      "onClick can be set only for View, Canvas2D components");
        if constexpr (V) {
            component_v.onClick(func);
        }
        if constexpr (C2) {
            component_2d.onClick(func);
        }
        return *this;
    }

    /*!
     * \since ver2.5
     */
    TemporalComponent &&onClick(const Func &func) && {
        this->onClick(func);
        return std::move(*this);
    }
    /*!
     * \brief クリック時に実行される関数を設定 (Viewまたは2D, FuncListener)
     * \since ver2.5
     *
     * 引数については ViewComponent::onClick(), Canvas2DComponent::onClick()
     * を参照
     *
     */
    TemporalComponent &onClick(const FuncListener &func) & {
        static_assert(V || C2,
                      "onClick can be set only for View, Canvas2D components");
        if constexpr (V) {
            component_v.onClick(func);
        }
        if constexpr (C2) {
            component_2d.onClick(func);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&onClick(const FuncListener &func) && {
        this->onClick(func);
        return std::move(*this);
    }

    /*!
     * \brief クリック時に実行される関数を設定 (Viewまたは2D)
     *
     * 引数については ViewComponent::onClick(), Canvas2DComponent::onClick()
     * を参照
     *
     */
    template <typename T, typename std::enable_if_t<std::is_invocable_v<T>,
                                                    std::nullptr_t> = nullptr>
    TemporalComponent &onClick(T func) & {
        static_assert(V || C2,
                      "onClick can be set only for View, Canvas2D components");
        auto func_shared =
            std::make_shared<std::function<void WEBCFACE_CALL_FP()>>(
                std::move(func));
        if constexpr (V) {
            component_v.onClick(func_shared);
        }
        if constexpr (C2) {
            component_2d.onClick(func_shared);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    template <typename T, typename std::enable_if_t<std::is_invocable_v<T>,
                                                    std::nullptr_t> = nullptr>
    TemporalComponent &&onClick(T func) && {
        this->onClick(std::move(func));
        return std::move(*this);
    }
    /*!
     * \brief 要素の移動 (2Dまたは3D)
     *
     */
    TemporalComponent &origin(const Transform &origin) & {
        static_assert(
            C2 || C3,
            "origin can be set only for Canvas2D, Canvas3D components");
        if constexpr (C2) {
            component_2d.origin(origin);
        }
        if constexpr (C3) {
            component_3d.origin(origin);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&origin(const Transform &origin) && {
        this->origin(origin);
        return std::move(*this);
    }
    /*!
     * \brief 色
     *
     * Viewの要素では textColor として設定される
     */
    TemporalComponent &color(ViewColor c) & {
        if constexpr (V) {
            component_v.textColor(c);
        }
        if constexpr (C2) {
            component_2d.color(c);
        }
        if constexpr (C3) {
            component_3d.color(c);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&color(ViewColor c) && {
        this->color(c);
        return std::move(*this);
    }
    /*!
     * \brief 表示する文字列 (View, Canvas2D)
     * \since ver2.0
     * 
     * ver2.10〜 StringInitializer 型で置き換え
     * 
     */
    TemporalComponent &text(const StringInitializer &str) & {
        static_assert(V || C2,
                      "text can be set only for View, Canvas2D components");
        if constexpr (V) {
            component_v.text(str);
        }
        if constexpr (C2) {
            component_2d.text(str);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     * 
     * ver2.10〜 StringInitializer 型で置き換え
     * 
     */
    TemporalComponent &&text(const StringInitializer &str) && {
        this->text(str);
        return std::move(*this);
    }
    /*!
     * \brief 文字色 (Viewまたは2D)
     *
     * Canvas2DのTextでは fillColor が文字色の代わりに使われている
     */
    TemporalComponent &textColor(ViewColor c) & {
        static_assert(
            V || C2, "textColor can be set only for View, Canvas2D components");
        if constexpr (V) {
            component_v.textColor(c);
        }
        if constexpr (C2) {
            component_2d.fillColor(c);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&textColor(ViewColor c) && {
        this->textColor(c);
        return std::move(*this);
    }
    /*!
     * \brief 背景色 (Viewまたは2D)
     *
     */
    TemporalComponent &fillColor(ViewColor c) & {
        static_assert(
            V || C2, "fillColor can be set only for View, Canvas2D components");
        if constexpr (V) {
            component_v.bgColor(c);
        }
        if constexpr (C2) {
            component_2d.fillColor(c);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&fillColor(ViewColor c) && {
        this->fillColor(c);
        return std::move(*this);
    }
    /*!
     * \brief 背景色 (Viewまたは2D)
     *
     */
    TemporalComponent &bgColor(ViewColor c) & {
        static_assert(V || C2,
                      "bgColor can be set only for View, Canvas2D components");
        return fillColor(c);
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&bgColor(ViewColor c) && {
        this->bgColor(c);
        return std::move(*this);
    }
    /*!
     * \brief 線の太さ (2Dのみ)
     *
     * 文字の太さではない
     */
    TemporalComponent &strokeWidth(double s) & {
        static_assert(C2,
                      "strokeWidth can be set only for Canvas2D components");
        if constexpr (C2) {
            component_2d.strokeWidth(s);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&strokeWidth(double s) && {
        this->strokeWidth(s);
        return std::move(*this);
    }
    /*!
     * \brief 文字の大きさ (2Dのみ)
     *
     */
    TemporalComponent &textSize(double s) & {
        static_assert(C2, "textSize can be set only for Canvas2D components");
        if constexpr (C2) {
            component_2d.textSize(s);
        }
        return *this;
    }
    /*!
     * \since ver2.5
     */
    TemporalComponent &&textSize(double s) && {
        this->textSize(s);
        return std::move(*this);
    }

    /*!
     * \brief 要素の幅 (Viewのみ)
     * \since ver2.6
     */
    TemporalComponent &width(int width) & {
        static_assert(V, "width can be set only for View components");
        if constexpr (V) {
            component_v.width(width);
        }
        return *this;
    }
    /*!
     * \brief 要素の幅 (Viewのみ)
     * \since ver2.6
     */
    TemporalComponent &&width(int width) && {
        this->width(width);
        return std::move(*this);
    }
    /*!
     * \brief 要素の高さ (Viewのみ)
     * \since ver2.6
     */
    TemporalComponent &height(int height) & {
        static_assert(V, "height can be set only for View components");
        if constexpr (V) {
            component_v.height(height);
        }
        return *this;
    }
    /*!
     * \brief 要素の高さ (Viewのみ)
     * \since ver2.6
     */
    TemporalComponent &&height(int height) && {
        this->height(height);
        return std::move(*this);
    }
};
class TemporalGeometry : public TemporalComponent<false, true, true>,
                         public Geometry {
  public:
    TemporalGeometry(GeometryType type, std::vector<double> &&properties)
        : TemporalComponent(nullptr, Canvas2DComponentType::geometry,
                            Canvas3DComponentType::geometry),
          Geometry(type, std::move(properties)) {
        this->component_2d.geometry(static_cast<Geometry &>(*this));
        this->component_3d.geometry(static_cast<Geometry &>(*this));
    }
};

inline namespace components {
inline namespace geometries {

struct Line {
    const Geometry &base;
    explicit Line(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 6) {
            throw InvalidArgument("Geometry type does not match: expected 6 "
                                  "properties for Line, got " +
                                  std::to_string(base.properties.size()));
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
inline TemporalGeometry line(const Point &begin, const Point &end) {
    return TemporalGeometry(GeometryType::line,
                            {begin.pos()[0], begin.pos()[1], begin.pos()[2],
                             end.pos()[0], end.pos()[1], end.pos()[2]});
}
struct Polygon {
    const Geometry &base;
    explicit Polygon(const Geometry &rg) : base(rg) {
        if (base.properties.size() % 3 != 0 || size() == 0) {
            throw InvalidArgument("Geometry type does not match: expected "
                                  "properties multiple of 3 for Polygon, got " +
                                  std::to_string(base.properties.size()));
        }
    }
    std::size_t size() const { return base.properties.size() / 3; }
    Point at(std::size_t i) const {
        if (i >= size()) {
            throw OutOfRange("Polygon::at(" + std::to_string(i) +
                             "), size = " + std::to_string(size()));
        }
        return Point{base.properties[i * 3 + 0], base.properties[i * 3 + 1],
                     base.properties[i * 3 + 2]};
    }
    Point operator[](std::size_t i) const { return at(i); }
};
inline TemporalGeometry polygon(const std::vector<Point> &points) {
    std::vector<double> properties;
    properties.reserve(points.size() * 3);
    for (const auto &p : points) {
        properties.push_back(p.pos(0));
        properties.push_back(p.pos(1));
        properties.push_back(p.pos(2));
    }
    return TemporalGeometry(GeometryType::polygon, std::move(properties));
}
struct Plane {
    const Geometry &base;
    explicit Plane(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 8) {
            throw InvalidArgument("Geometry type does not match: expected 8 "
                                  "properties for Plane, got " +
                                  std::to_string(base.properties.size()));
        }
    }
    Transform origin() const {
        return Transform(
            {base.properties[0], base.properties[1], base.properties[2]},
            rotFromEuler(base.properties[3], base.properties[4],
                         base.properties[5]));
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
inline TemporalGeometry plane(const Transform &origin, double width,
                              double height) {
    return TemporalGeometry(GeometryType::plane,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             origin.rotEuler()[0], origin.rotEuler()[1],
                             origin.rotEuler()[2], width, height});
}
using Rect = Plane;
inline TemporalGeometry rect(const Point &origin, double width, double height) {
    return TemporalGeometry{GeometryType::plane,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             0, 0, 0, width, height}};
}
inline TemporalGeometry rect(const Point &p1, const Point &p2) {
    Transform origin = identity();
    for (int i = 0; i < 2; i++) {
        origin.pos(i) = (p1.pos(i) + p2.pos(i)) / 2;
    }
    double width = std::abs(p1.pos(0) - p2.pos(0));
    double height = std::abs(p1.pos(0) - p2.pos(0));
    return TemporalGeometry{GeometryType::plane,
                            {origin.pos(0), origin.pos(1), origin.pos(2),
                             origin.rotEuler()[0], origin.rotEuler()[1],
                             origin.rotEuler()[2], width, height}};
}

struct Box {
    const Geometry &base;
    explicit Box(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 6) {
            throw InvalidArgument("Geometry type does not match: expected 6 "
                                  "properties for Box, got " +
                                  std::to_string(base.properties.size()));
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
inline TemporalGeometry box(const Point &vertex1, const Point &vertex2) {
    return TemporalGeometry{GeometryType::box,
                            {vertex1.pos()[0], vertex1.pos()[1],
                             vertex1.pos()[2], vertex2.pos()[0],
                             vertex2.pos()[1], vertex2.pos()[2]}};
}

struct Circle {
    const Geometry &base;
    explicit Circle(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 7) {
            throw InvalidArgument("Geometry type does not match: expected 7 "
                                  "properties for Circle, got " +
                                  std::to_string(base.properties.size()));
        }
    }
    Transform origin() const {
        return Transform(
            {base.properties[0], base.properties[1], base.properties[2]},
            rotFromEuler(base.properties[3], base.properties[4],
                         base.properties[5]));
    }
    double radius() const { return base.properties[6]; }
};
inline TemporalGeometry circle(const Transform &origin, double radius) {
    return TemporalGeometry{GeometryType::circle,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             origin.rotEuler()[0], origin.rotEuler()[1],
                             origin.rotEuler()[2], radius}};
}
inline TemporalGeometry circle(const Point &origin, double radius) {
    return TemporalGeometry{
        GeometryType::circle,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], 0, 0, 0, radius}};
}

struct Cylinder {
    const Geometry &base;
    explicit Cylinder(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 8) {
            throw InvalidArgument("Geometry type does not match: expected 8 "
                                  "properties for Cylinder, got " +
                                  std::to_string(base.properties.size()));
        }
    }
    Transform origin() const {
        return Transform(
            {base.properties[0], base.properties[1], base.properties[2]},
            rotFromEuler(base.properties[3], base.properties[4],
                         base.properties[5]));
    }
    double radius() const { return base.properties[6]; }
    double length() const { return base.properties[7]; }
};
inline TemporalGeometry cylinder(const Transform &origin, double radius,
                                 double length) {
    return TemporalGeometry{GeometryType::cylinder,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             origin.rotEuler()[0], origin.rotEuler()[1],
                             origin.rotEuler()[2], radius, length}};
}

struct Sphere {
    const Geometry &base;
    explicit Sphere(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 4) {
            throw InvalidArgument("Geometry type does not match: expected 4 "
                                  "properties for Sphere, got " +
                                  std::to_string(base.properties.size()));
        }
    }
    Point origin() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    double radius() const { return base.properties[3]; }
};
inline TemporalGeometry sphere(const Point &origin, double radius) {
    return TemporalGeometry{
        GeometryType::sphere,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], radius}};
}


} // namespace geometries

namespace Geometries = geometries; // 〜ver1.11

/*!
 * \brief textコンポーネント
 *
 * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
 * 
 */
inline TemporalComponent<true, true, false> text(const StringInitializer &text) {
    return TemporalComponent<true, true, false>(
               ViewComponentType::text, Canvas2DComponentType::text, nullptr)
        .text(text);
}
/*!
 * \brief newLineコンポーネント
 *
 */
inline TemporalViewComponent newLine() {
    return TemporalViewComponent(ViewComponentType::new_line);
}

/*!
 * \brief buttonコンポーネント
 *
 * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
 * 
 */
template <typename T>
inline TemporalViewComponent button(const StringInitializer &text, T &&func) {
    return TemporalViewComponent(ViewComponentType::button)
        .text(text)
        .onClick(std::forward<T>(func));
}
/*!
 * \brief buttonコンポーネント (wstring)
 * \since ver2.0
 */
template <typename T>
inline TemporalViewComponent button(std::wstring_view text, T &&func) {
    return TemporalViewComponent(ViewComponentType::button)
        .text(text)
        .onClick(std::forward<T>(func));
}

inline TemporalViewComponent textInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::text_input).text(text);
}
inline TemporalViewComponent decimalInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::decimal_input)
        .text(text)
        .init(0);
}
inline TemporalViewComponent numberInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::number_input)
        .text(text)
        .init(0);
}
inline TemporalViewComponent toggleInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::toggle_input).text(text);
}
inline TemporalViewComponent selectInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::select_input).text(text);
}
inline TemporalViewComponent sliderInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::slider_input)
        .text(text)
        .init(0);
}
inline TemporalViewComponent checkInput(const StringInitializer &text = {}) {
    return TemporalViewComponent(ViewComponentType::check_input)
        .text(text)
        .init(false);
}
} // namespace components

namespace Components = components;     // 〜ver1.11
namespace ViewComponents = components; // 〜ver1.8

WEBCFACE_NS_END
