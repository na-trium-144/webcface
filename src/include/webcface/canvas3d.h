#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include "func.h"
#include "event_target.h"
#include "common/def.h"
#include "common/canvas3d.h"
#include "common/robot_model.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
inline namespace Geometries {
struct Line : Geometry {
    Line(const Transform &begin, const Transform &end)
        : Geometry(GeometryType::line,
                   {begin.pos()[0], begin.pos()[1], begin.pos()[2],
                    end.pos()[0], end.pos()[1], end.pos()[2]}) {}
    Line(const Geometry &rg) : Geometry(rg) {
        if (properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform begin() const {
        return Transform{properties[0], properties[1], properties[2], 0, 0, 0};
    }
    Transform end() const {
        return Transform{properties[3], properties[4], properties[5], 0, 0, 0};
    }
};
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
    double width() const { return properties[0]; }
    double height() const { return properties[1]; }
};
} // namespace Geometries

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
    explicit Canvas3DComponent(Canvas3DComponentType type) { type_ = type; }
};

/*!
 * \brief Canvas3Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas3D() を使って取得してください
 *
 */
class Canvas3D : protected Field, public EventTarget<Canvas3D> {
    std::shared_ptr<std::vector<Canvas3DComponent>> components;
    bool modified;

    WEBCFACE_DLL void onAppend() const override;

    /*!
     * \brief 値をセットし、EventTargetを発動する
     *
     */
    WEBCFACE_DLL Canvas3D &set(std::vector<Canvas3DComponent> &v);

    WEBCFACE_DLL void onDestroy();

  public:
    WEBCFACE_DLL Canvas3D();
    WEBCFACE_DLL Canvas3D(const Field &base);
    Canvas3D(const Field &base, const std::string &field)
        : Canvas3D(Field{base, field}) {}
    Canvas3D(const Canvas3D &rhs) : Canvas3D() { *this = rhs; }
    WEBCFACE_DLL Canvas3D &operator=(const Canvas3D &rhs);
    WEBCFACE_DLL Canvas3D &operator=(Canvas3D &&rhs);

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
    std::vector<ViewComponent> get() const {
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
     * \brief コンポーネントを追加
     *
     */
    Canvas3D &operator<<(const Common::Canvas3DComponentBase &vc) {
        *this << Canvas3DComponent{vc, this->data_w};
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     */
    WEBCFACE_DLL Canvas3D &operator<<(const Canvas3DComponent &vc);

    /*!
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     */
    template <typename T>
    Canvas3D &add(const T &rhs) {
        *this << rhs;
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * * ver1.2以降: このViewオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    WEBCFACE_DLL Canvas3D &sync();
};
} // namespace WEBCFACE_NS
