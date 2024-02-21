#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include <utility>
#include <stdexcept>
#include <concepts>
#include "func.h"
#include "event_target.h"
#include "common/def.h"
#include "common/canvas3d.h"
#include "common/robot_model.h"
#include "robot_model.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
inline namespace Geometries {
struct Line : Geometry, Geometry3D, Geometry2D {
    Line(const Point &begin, const Point &end)
        : Geometry(GeometryType::line,
                   {begin.pos()[0], begin.pos()[1], begin.pos()[2],
                    end.pos()[0], end.pos()[1], end.pos()[2]}) {}
    Line(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point begin() const {
        return Point{properties[0], properties[1], properties[2]};
    }
    Point end() const {
        return Point{properties[3], properties[4], properties[5]};
    }
};
inline Line line(const Point &begin, const Point &end) {
    return Line(begin, end);
}
struct Polygon : Geometry, Geometry3D, Geometry2D {
    Polygon(const std::vector<Point> &points)
        : Geometry(GeometryType::polygon, {}) {
        for (const auto &p : points) {
            properties.push_back(p.pos(0));
            properties.push_back(p.pos(1));
            properties.push_back(p.pos(2));
        }
    }
    Polygon(const Geometry &rg) : Geometry(rg) {
        if (properties.size() % 3 != 0 || size() == 0) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    std::size_t size() const { return properties.size() / 3; }
    Point at(std::size_t i) const {
        if (i >= size()) {
            throw std::out_of_range("Polygon::at(" + std::to_string(i) +
                                    "), size = " + std::to_string(size()));
        }
        return Point{properties[i * 3 + 0], properties[i * 3 + 1],
                     properties[i * 3 + 2]};
    }
    Point operator[](std::size_t i) const { return at(i); }
};
inline Polygon polygon(const std::vector<Point> &points) {
    return Polygon(points);
}
struct Plane : Geometry, Geometry3D, Geometry2D {
    Plane(const Transform &origin, double width, double height)
        : Geometry(GeometryType::plane,
                   {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                    origin.rot()[0], origin.rot()[1], origin.rot()[2], width,
                    height}) {}
    /*!
     * ver1.6で追加
     *
     */
    Plane(const Point &origin, double width, double height)
        : Geometry(GeometryType::plane,
                   {origin.pos()[0], origin.pos()[1], origin.pos()[2], 0, 0, 0,
                    width, height}) {}
    /*!
     * ver1.6で追加
     *
     */
    Plane(const Point &p1, const Point &p2)
        : Geometry(GeometryType::plane, {}) {
        Transform origin = identity();
        for (int i = 0; i < 2; i++) {
            origin.pos(i) = (p1.pos(i) + p2.pos(i)) / 2;
        }
        double width = std::abs(p1.pos(0) - p2.pos(0));
        double height = std::abs(p1.pos(0) - p2.pos(0));
        properties = {origin.pos(0), origin.pos(1), origin.pos(2),
                      origin.rot(0), origin.rot(1), origin.rot(2),
                      width,         height};
    }
    Plane(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 8) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{properties[0], properties[1], properties[2],
                         properties[3], properties[4], properties[5]};
    }
    double width() const { return properties[6]; }
    double height() const { return properties[7]; }
    /*!
     * todo: 3次元のplaneの場合正しくない
     *
     */
    Point vertex1() const {
        return {properties[0] - width() / 2, properties[1] - height() / 2,
                properties[2]};
    }
    Point vertex2() const {
        return {properties[0] + width() / 2, properties[1] + height() / 2,
                properties[2]};
    }
};
inline Plane plane(const Transform &origin, double width, double height) {
    return Plane(origin, width, height);
}
using Rect = Plane;
inline Rect rect(const Point &origin, double width, double height) {
    return Rect(origin, width, height);
}
inline Rect rect(const Point &p1, const Point &p2) { return Rect(p1, p2); }

struct Box : Geometry, Geometry3D {
    Box(const Point &vertex1, const Point &vertex2)
        : Geometry(GeometryType::box,
                   {vertex1.pos()[0], vertex1.pos()[1], vertex1.pos()[2],
                    vertex2.pos()[0], vertex2.pos()[1], vertex2.pos()[2]}) {}
    Box(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point vertex1() const {
        return Point{properties[0], properties[1], properties[2]};
    }
    Point vertex2() const {
        return Point{properties[3], properties[4], properties[5]};
    }
};
inline Box box(const Point &vertex1, const Point &vertex2) {
    return Box(vertex1, vertex2);
}

struct Circle : Geometry, Geometry3D, Geometry2D {
    Circle(const Transform &origin, double radius)
        : Geometry(GeometryType::circle,
                   {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                    origin.rot()[0], origin.rot()[1], origin.rot()[2],
                    radius}) {}
    /*!
     * ver1.6で追加
     *
     */
    Circle(const Point &origin, double radius)
        : Geometry(GeometryType::circle, {origin.pos()[0], origin.pos()[1],
                                          origin.pos()[2], 0, 0, 0, radius}) {}
    Circle(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 7) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{properties[0], properties[1], properties[2],
                         properties[3], properties[4], properties[5]};
    }
    double radius() const { return properties[6]; }
};
inline Circle circle(const Transform &origin, double radius) {
    return Circle(origin, radius);
}
inline Circle circle(const Point &origin, double radius) {
    return Circle(origin, radius);
}

struct Cylinder : Geometry, Geometry3D {
    Cylinder(const Transform &origin, double radius, double length)
        : Geometry(GeometryType::cylinder,
                   {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                    origin.rot()[0], origin.rot()[1], origin.rot()[2], radius,
                    length}) {}
    Cylinder(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 8) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{properties[0], properties[1], properties[2],
                         properties[3], properties[4], properties[5]};
    }
    double radius() const { return properties[6]; }
    double length() const { return properties[7]; }
};
inline Cylinder cylinder(const Transform &origin, double radius,
                         double length) {
    return Cylinder{origin, radius, length};
}

struct Sphere : Geometry, Geometry3D {
    Sphere(const Point &origin, double radius)
        : Geometry(GeometryType::sphere, {origin.pos()[0], origin.pos()[1],
                                          origin.pos()[2], radius}) {}
    Sphere(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 4) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point origin() const {
        return Point{properties[0], properties[1], properties[2]};
    }
    double radius() const { return properties[3]; }
};
inline Sphere sphere(const Point &origin, double radius) {
    return Sphere{origin, radius};
}

} // namespace Geometries

/*!
 * \brief Canvas3Dに表示する要素
 *
 */
class Canvas3DComponent : public Common::Canvas3DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;

  public:
    Canvas3DComponent() = default;
    Canvas3DComponent(const Common::Canvas3DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas3DComponentBase(vc), data_w(data_w) {}
    explicit Canvas3DComponent(Canvas3DComponentType type) { type_ = type; }
};

/*!
 * \brief Canvas3Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas3D() を使って取得してください
 *
 */
class Canvas3D : protected Field, public EventTarget<Canvas3D> {
    std::shared_ptr<std::vector<Canvas3DComponentBase>> components;
    std::shared_ptr<bool> modified;

    WEBCFACE_DLL void onAppend() const override;

    /*!
     * \brief 値をセットし、EventTargetを発動する
     *
     */
    WEBCFACE_DLL Canvas3D &set(std::vector<Canvas3DComponentBase> &v);

    WEBCFACE_DLL void onDestroy();

  public:
    WEBCFACE_DLL Canvas3D();
    WEBCFACE_DLL Canvas3D(const Field &base);
    Canvas3D(const Field &base, const std::string &field)
        : Canvas3D(Field{base, field}) {}

    /*!
     * \brief デストラクタで sync() を呼ぶ。
     *
     * Canvas3Dをコピーした場合は、すべてのコピーが破棄されたときにのみ sync()
     * が呼ばれる。
     * \sa sync()
     *
     */
    ~Canvas3D() override { onDestroy(); }

    using Field::member;
    using Field::name;

    /*!
     * \brief 子フィールドを返す
     *
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするCanvas3D
     *
     */
    Canvas3D child(const std::string &field) const {
        return Canvas3D{*this, this->field_ + "." + field};
    }
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    WEBCFACE_DLL std::optional<std::vector<Canvas3DComponent>> tryGet() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::vector<Canvas3DComponent> get() const {
        return tryGet().value_or(std::vector<Canvas3DComponent>{});
    }
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     */
    [[deprecated]] WEBCFACE_DLL std::chrono::system_clock::time_point
    time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    WEBCFACE_DLL Canvas3D &free();

    /*!
     * \brief このCanvas3Dに追加した内容を初期化する
     *
     * このCanvas3Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas3Dが送信される)
     *
     */
    WEBCFACE_DLL Canvas3D &init();

    /*!
     * \brief Componentを追加
     *
     */
    WEBCFACE_DLL Canvas3D &add(const Canvas3DComponentBase &cc);

    /*!
     * \brief Geometryを追加
     * \param geometry 表示する図形
     * \param origin geometryを移動する
     * \param color 表示色 (省略時のinheritはWebUI上ではgrayと同じ)
     *
     */
    template <typename G>
        requires std::derived_from<G, Geometry3D>
    Canvas3D &add(const G &geometry, const Transform &origin,
                  const ViewColor &color = ViewColor::inherit) {
        add({Canvas3DComponentType::geometry,
             origin,
             color,
             geometry,
             std::nullopt,
             {}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * originを省略した場合 identity() になる
     *
     */
    template <typename G>
        requires std::derived_from<G, Geometry3D>
    Canvas3D &add(const G &geometry,
                  const ViewColor &color = ViewColor::inherit) {
        add(geometry, identity(), color);
        return *this;
    }
    /*!
     * \brief RobotModelを追加
     *
     * jointのangleを変更できる。
     * それ以外のパラメータは元のモデルのまま。
     *
     */
    WEBCFACE_DLL Canvas3D &add(const RobotModel &model_field,
                               const Transform &origin,
                               std::unordered_map<std::string, double> angles) {
        std::unordered_map<std::size_t, double> angles_i;
        auto model = model_field.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            if (angles.count(j.name)) {
                angles_i[ji] = angles[j.name];
            }
        }
        add({Canvas3DComponentType::robot_model, origin, ViewColor::inherit,
             std::nullopt, static_cast<FieldBase>(model_field), angles_i});
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * このCanvas3Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    WEBCFACE_DLL Canvas3D &sync();
};
} // namespace WEBCFACE_NS
