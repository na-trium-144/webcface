#pragma once
#include <concepts>
#include <memory>
#include "common/def.h"
#include "common/canvas2d.h"
#include "event_target.h"
#include "field.h"

namespace WEBCFACE_NS {

class Canvas2DComponent : public Common::Canvas2DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;

  public:
    Canvas2DComponent() = default;
    Canvas2DComponent(const Common::Canvas2DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas2DComponentBase(vc), data_w(data_w) {}
    explicit Canvas2DComponent(Canvas2DComponentType type) { type_ = type; }
};
/*!
 * \brief Canvas2Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas2D() を使って取得してください
 *
 */
class Canvas2D : protected Field, public EventTarget<Canvas2D> {
    std::shared_ptr<Canvas2DData> canvas_data;
    std::shared_ptr<bool> modified;

    WEBCFACE_DLL void onAppend() const override;

    /*!
     * \brief 値をセットし、EventTargetを発動する
     *
     */
    WEBCFACE_DLL Canvas2D &set(Canvas2DData &v);

    WEBCFACE_DLL void onDestroy();

  public:
    WEBCFACE_DLL Canvas2D();
    WEBCFACE_DLL Canvas2D(const Field &base);
    Canvas2D(const Field &base, const std::string &field)
        : Canvas2D(Field{base, field}) {}
    Canvas2D(const Field &base, const std::string &field, double width,
             double height)
        : Canvas2D(Field{base, field}) {
        init(width, height);
    }

    /*!
     * \brief デストラクタで sync() を呼ぶ。
     *
     * Canvas2Dをコピーした場合は、すべてのコピーが破棄されたときにのみ sync()
     * が呼ばれる。
     * \sa sync()
     *
     */
    ~Canvas2D() override { onDestroy(); }

    using Field::member;
    using Field::name;

    /*!
     * \brief 子フィールドを返す
     *
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするCanvas3D
     *
     */
    Canvas2D child(const std::string &field) const {
        return Canvas2D{*this, this->field_ + "." + field};
    }
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    WEBCFACE_DLL std::optional<std::vector<Canvas2DComponent>> tryGet() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::vector<Canvas2DComponent> get() const {
        return tryGet().value_or(std::vector<Canvas2DComponent>{});
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
    WEBCFACE_DLL Canvas2D &free();

    /*!
     * \brief Canvasのサイズを指定 & このCanvas2Dに追加した内容を初期化する
     *
     * このCanvas2Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas2Dが送信される)
     *
     */
    WEBCFACE_DLL Canvas2D &init(double width, double height);

    /*!
     * \brief Componentを追加
     *
     */
    WEBCFACE_DLL Canvas2D &add(const Canvas2DComponentBase &cc);

    /*!
     * \brief Geometryを追加
     *
     * 事前に init() かコンストラクタでCanvasのサイズを指定しないと
     * std::invalid_argument を投げます
     * \param geometry 表示したい図形
     * \param origin geometryを移動する
     * \param color 図形の枠線の色 (省略時のinheritはWebUI上ではblackと同じ)
     * \param fill 塗りつぶしの色 (省略時のinheritはWebUI上では透明)
     * \param stroke_width 枠線の太さ
     *
     */
    template <typename G>
        requires std::derived_from<G, Geometry2D>
    Canvas2D &add(const G &geometry, const Transform &origin,
                  const ViewColor &color = ViewColor::inherit,
                  const ViewColor &fill = ViewColor::inherit,
                  double stroke_width = 1) {
        add({Canvas2DComponentType::geometry, origin, color, fill, stroke_width,
             geometry});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * origin を省略した場合identity()になる
     *
     */
    template <typename G>
        requires std::derived_from<G, Geometry2D>
    Canvas2D &
    add(const G &geometry, const ViewColor &color = ViewColor::inherit,
        const ViewColor &fill = ViewColor::inherit, double stroke_width = 1) {
        add({Canvas2DComponentType::geometry, identity(), color, fill,
             stroke_width, geometry});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * fillを省略
     *
     */
    template <typename G>
        requires std::derived_from<G, Geometry2D>
    Canvas2D &add(const G &geometry, const Transform &origin,
                  const ViewColor &color, double stroke_width) {
        add({Canvas2DComponentType::geometry, origin, color, ViewColor::inherit,
             stroke_width, geometry});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * originとfillを省略
     *
     */
    template <typename G>
        requires std::derived_from<G, Geometry2D>
    Canvas2D &add(const G &geometry, const ViewColor &color,
                  double stroke_width) {
        add({Canvas2DComponentType::geometry, color, ViewColor::inherit,
             stroke_width, geometry});
        return *this;
    }
    /*!
     * \brief 内容をclientに反映し送信可能にする
     *
     * このCanvas2Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    WEBCFACE_DLL Canvas2D &sync();
};
} // namespace WEBCFACE_NS
