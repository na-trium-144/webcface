#pragma once
#include <concepts>
#include <memory>
#include "common/def.h"
#include "common/canvas2d.h"
#include "event_target.h"
#include "field.h"
#include "func.h"
#include "canvas_data.h"

namespace WEBCFACE_NS {
namespace Internal {
template <typename Component>
class DataSetBuffer;
class Canvas2DDataBuf;
}

class Canvas2D;
extern template class WEBCFACE_IMPORT EventTarget<Canvas2D>;

/*!
 * \brief Canvas2Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas2D() を使って取得してください
 *
 */
class WEBCFACE_DLL Canvas2D : protected Field, public EventTarget<Canvas2D> {
    std::shared_ptr<Internal::Canvas2DDataBuf> sb;

    void onAppend() const override;

  public:
    Canvas2D();
    Canvas2D(const Field &base);
    Canvas2D(const Field &base, const std::string &field)
        : Canvas2D(Field{base, field}) {}
    Canvas2D(const Field &base, const std::string &field, double width,
             double height)
        : Canvas2D(Field{base, field}) {
        init(width, height);
    }

    using Field::member;
    using Field::name;
    friend Internal::DataSetBuffer<Canvas2DComponent>;

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
     * \brief Canvasの内容をリクエストする
     * \since ver1.7
     *
     */
    void request() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::optional<std::vector<Canvas2DComponent>> tryGet() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::vector<Canvas2DComponent> get() const {
        return tryGet().value_or(std::vector<Canvas2DComponent>{});
    }
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point
    time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    Canvas2D &free();

    /*!
     * \brief Canvasのサイズを指定 & このCanvas2Dに追加した内容を初期化する
     *
     * このCanvas2Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas2Dが送信される)
     *
     */
    Canvas2D &init(double width, double height);

    /*!
     * \brief Componentを追加
     * \since ver1.9
     * 
     */
    Canvas2D &operator<<(const Canvas2DComponent &cc);
    /*!
     * \brief Componentを追加
     * \since ver1.9
     * 
     */
    Canvas2D &operator<<(Canvas2DComponent &&cc);
    /*!
     * \brief Componentを追加
     *
     */
    Canvas2D &add(const Canvas2DComponent &cc){
        *this << cc;
        return *this;
    }
    /*!
     * \brief Componentを追加
     *
     */
    Canvas2D &add(Canvas2DComponent &&cc){
        *this << std::move(cc);
        return *this;
    }

    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    Canvas2D &add(CanvasCommonComponent &&cc) {
        *this << std::move(cc.to2());
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    Canvas2D &add(CanvasCommonComponent &cc) {
        *this << cc.to2();
        return *this;
    }
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
     * \deprecated 1.9〜
     * CanvasCommonComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const Transform &origin,
                                 const ViewColor &color = ViewColor::inherit,
                                 const ViewColor &fill = ViewColor::inherit,
                                 double stroke_width = 1) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, origin, color,
                               fill, stroke_width, geometry, std::nullopt}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * origin を省略した場合identity()になる
     *
     * \deprecated 1.9〜
     * CanvasCommonComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const ViewColor &color = ViewColor::inherit,
                                 const ViewColor &fill = ViewColor::inherit,
                                 double stroke_width = 1) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, identity(),
                               color, fill, stroke_width, geometry,
                               std::nullopt}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * fillを省略
     *
     * \deprecated 1.9〜
     * CanvasCommonComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const Transform &origin,
                                 const ViewColor &color, double stroke_width) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, origin, color,
                               ViewColor::inherit, stroke_width, geometry,
                               std::nullopt}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * originとfillを省略
     *
     * \deprecated 1.9〜
     * CanvasCommonComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const ViewColor &color, double stroke_width) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, identity(),
                               color, ViewColor::inherit, stroke_width,
                               geometry, std::nullopt}});
        return *this;
    }
    /*!
     * \brief 内容をclientに反映し送信可能にする
     *
     * このCanvas2Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    Canvas2D &sync();
};
} // namespace WEBCFACE_NS
