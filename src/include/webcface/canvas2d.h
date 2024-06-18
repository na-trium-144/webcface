#pragma once
#include <concepts>
#include <memory>
#include <webcface/common/def.h>
#include "field.h"
#include "canvas_data.h"

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename Component>
class DataSetBuffer;
class Canvas2DDataBuf;
} // namespace Internal

/*!
 * \brief Canvas2Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas2D() を使って取得してください
 *
 */
class WEBCFACE_DLL Canvas2D : protected Field, public EventTarget<Canvas2D> {
    std::shared_ptr<Internal::Canvas2DDataBuf> sb;

    void onAppend() const override final;

  public:
    Canvas2D();
    Canvas2D(const Field &base);
    Canvas2D(const Field &base, const SharedString &field)
        : Canvas2D(Field{base, field}) {}
    Canvas2D(const Field &base, const SharedString &field, double width,
             double height)
        : Canvas2D(Field{base, field}) {
        init(width, height);
    }

    friend Internal::DataSetBuffer<Canvas2DComponent>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Canvas2D child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Canvas2D child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Canvas2D child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Canvas2D operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    Canvas2D operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Canvas2D operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Canvas2D operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Canvas2D operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Canvas2D parent() const { return this->Field::parent(); }

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
    [[deprecated]] std::chrono::system_clock::time_point time() const;

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
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     * \since ver1.9〜
     *
     */
    template <typename T>
    Canvas2D &add(T &&cc) {
        *this << std::forward<T>(cc);
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    template <bool V, bool C3>
    Canvas2D &operator<<(TemporalComponent<V, true, C3> &&cc) {
        *this << std::move(cc.to2());
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    template <bool V, bool C3>
    Canvas2D &operator<<(TemporalComponent<V, true, C3> &cc) {
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
     * TemporalComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const Transform &origin,
                                 const ViewColor &color = ViewColor::inherit,
                                 const ViewColor &fill = ViewColor::inherit,
                                 double stroke_width = 1) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, origin, color,
                               fill, stroke_width, geometry, std::nullopt,
                               nullptr}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * origin を省略した場合identity()になる
     *
     * \deprecated 1.9〜
     * TemporalComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const ViewColor &color = ViewColor::inherit,
                                 const ViewColor &fill = ViewColor::inherit,
                                 double stroke_width = 1) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, identity(),
                               color, fill, stroke_width, geometry,
                               std::nullopt, nullptr}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * fillを省略
     *
     * \deprecated 1.9〜
     * TemporalComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const Transform &origin,
                                 const ViewColor &color, double stroke_width) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, origin, color,
                               ViewColor::inherit, stroke_width, geometry,
                               std::nullopt, nullptr}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * originとfillを省略
     *
     * \deprecated 1.9〜
     * TemporalComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas2D &add(const Geometry &geometry,
                                 const ViewColor &color, double stroke_width) {
        add(Canvas2DComponent{{Canvas2DComponentType::geometry, identity(),
                               color, ViewColor::inherit, stroke_width,
                               geometry, std::nullopt, nullptr}});
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

    /*!
     * \brief Canvas2Dの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, Canvas2D>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
