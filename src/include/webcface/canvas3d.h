#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include <utility>
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
struct Line : Geometry {
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
struct Plane : Geometry {
    Plane(const Transform &origin, double width, double height)
        : Geometry(GeometryType::plane,
                   {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                    origin.rot()[0], origin.rot()[1], origin.rot()[2], width,
                    height}) {}
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
};
inline Plane plane(const Transform &origin, double width, double height) {
    return Plane(origin, width, height);
}

struct Box : Geometry {
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
    return Box{vertex1, vertex2};
}

struct Circle : Geometry {
    Circle(const Transform &origin, double radius)
        : Geometry(GeometryType::circle,
                   {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                    origin.rot()[0], origin.rot()[1], origin.rot()[2],
                    radius}) {}
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
    return Circle{origin, radius};
}

struct Cylinder : Geometry {
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

struct Sphere : Geometry {
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
     *
     */
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

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
     *
     */
    WEBCFACE_DLL Canvas3D &add(const Geometry &geometry,
                               const Transform &origin,
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
     * originを省略した場合 identity()
     *
     */
    WEBCFACE_DLL Canvas3D &add(const Geometry &geometry,
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
