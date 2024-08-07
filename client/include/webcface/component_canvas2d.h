#pragma once
#include <vector>
#include <optional>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "transform.h"
#include "field.h"
#include "webcface/component_view.h"
#include "webcface/geometry.h"

WEBCFACE_NS_BEGIN
namespace message {
struct Canvas2DComponent;
}

/*!
 * \brief Canvas2Dの各要素を表すクラス。
 *
 * Geometryをセットするときは TemporalComponent を使うので、
 * 今のところこのクラスのオブジェクトのデータを変更する用途はない。
 *
 */
class WEBCFACE_DLL Canvas2DComponent : public IdBase<Canvas2DComponentType> {
    std::weak_ptr<internal::ClientData> data_w;
    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

    Canvas2DComponentType type_;
    Transform origin_;
    ViewColor color_, fill_;
    double stroke_width_ = 0;
    std::optional<Geometry> geometry_;
    std::optional<FieldBase> on_click_func_;
    SharedString text_;

  public:
    Canvas2DComponent() = default;
    Canvas2DComponent(Canvas2DComponentType type, const Transform &origin,
                      ViewColor color, ViewColor fill, double stroke_width,
                      std::optional<Geometry> &&geometry,
                      std::optional<FieldBase> &&on_click_func,
                      const SharedString &text)
        : type_(type), origin_(origin), color_(color), fill_(fill),
          stroke_width_(stroke_width), geometry_(std::move(geometry)),
          on_click_func_(std::move(on_click_func)), text_(text) {}
    Canvas2DComponent(const Canvas2DComponent &vc,
                      const std::weak_ptr<internal::ClientData> &data_w,
                      std::unordered_map<int, int> *idx_next)
        : Canvas2DComponent(vc) {
        this->data_w = data_w;
        initIdx(idx_next, type_);
    }
    explicit Canvas2DComponent(Canvas2DComponentType type)
        : IdBase<Canvas2DComponentType>(), data_w(),
          on_click_func_tmp(nullptr) {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncの名前を確定
     *
     */
    Canvas2DComponent &
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<int, int> *idx_next = nullptr);

    message::Canvas2DComponent toMessage() const;
    Canvas2DComponent(const message::Canvas2DComponent &cc);

    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas2DComponent &other) const {
        return id() == other.id() && type_ == other.type_ &&
               origin_ == other.origin_ && color_ == other.color_ &&
               fill_ == other.fill_ && stroke_width_ == other.stroke_width_ &&
               geometry_ == other.geometry_ &&
               on_click_func_ == other.on_click_func_ && text_ == other.text_;
    }
    /*!
     * \since ver1.11
     */
    bool operator!=(const Canvas2DComponent &other) const {
        return !(*this == other);
    }

    /*!
     * \brief 要素の種類
     *
     */
    Canvas2DComponentType type() const override { return type_; }
    /*!
     * \brief 要素の移動
     *
     * 要素を平行移動&回転します。
     *
     */
    Canvas2DComponent &origin(const Transform &origin) {
        origin_ = origin;
        return *this;
    }
    Transform origin() const { return origin_; }
    /*!
     * \brief 色
     *
     * 図形の輪郭の色を指定します。
     * デフォルト時のinheritはWebUI上ではblackとして表示されます
     *
     */
    Canvas2DComponent &color(const ViewColor &color) {
        color_ = color;
        return *this;
    }
    ViewColor color() const { return color_; }
    /*!
     * \brief 塗りつぶし色
     *
     * 図形の塗りつぶし色を指定します。
     * デフォルト時のinheritはWebUI上では透明になります
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
     * 図形の輪郭の太さを指定します。
     * 太さ1はCanvas2Dの座標系で1の長さ分の太さになります(拡大縮小で太さが変わる)
     *
     * 指定しない場合0となり、WebUIではその場合Canvasの拡大に関係なく1ピクセルになります
     *
     */
    Canvas2DComponent &strokeWidth(double s) {
        stroke_width_ = s;
        return *this;
    }
    double strokeWidth() const { return stroke_width_; }
    /*!
     * \brief 文字の大きさ(高さ)
     * \since ver1.9
     *
     * 文字の大きさを指定します(Text要素の場合のみ)
     * 大きさ1は文字の高さがCanvas2Dの座標系で1の長さ分になります(拡大縮小で大きさが変わる)
     *
     * 内部のデータとしてはstrokeWidthのデータを使いまわしています
     *
     */
    Canvas2DComponent &textSize(double s) { return strokeWidth(s); }
    double textSize() const { return stroke_width_; }
    /*!
     * \brief 表示する文字列
     * \since ver1.9
     */
    std::string text() const { return text_.decode(); }
    /*!
     * \brief 表示する文字列 (wstring)
     * \since ver2.0
     */
    std::wstring textW() const { return text_.decodeW(); }
    /*!
     * \brief 表示する文字列を設定
     * \since ver1.9
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    Canvas2DComponent &text(std::string_view text) {
        text_ = SharedString::encode(text);
        return *this;
    }
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    Canvas2DComponent &text(std::wstring_view text) {
        text_ = SharedString::encode(text);
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
    Canvas2DComponent &geometry(const Geometry &g) {
        geometry_.emplace(g);
        return *this;
    };
    /*!
     * \brief クリック時に実行される関数を取得
     * \since ver1.9
     */
    std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    Canvas2DComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func 実行する任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    Canvas2DComponent &onClick(T func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (コピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    Canvas2DComponent &onClick(AnonymousFunc &&func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(std::move(func));
        return *this;
    }
};

struct Canvas2DDataBase {
    double width = 0, height = 0;
    std::vector<Canvas2DComponent> components;
    Canvas2DDataBase() = default;
    Canvas2DDataBase(double width, double height)
        : width(width), height(height), components() {}
    Canvas2DDataBase(double width, double height,
                     std::vector<Canvas2DComponent> &&components)
        : width(width), height(height), components(std::move(components)) {}
};


WEBCFACE_NS_END
