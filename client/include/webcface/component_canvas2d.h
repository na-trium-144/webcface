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
namespace internal {
struct Canvas2DComponentData;
}

enum class Canvas2DComponentType {
    geometry = 0,
    text = 3,
};
/*!
 * \brief Canvas2Dの各要素を表すクラス。
 *
 * * ver2.0〜:
 * get専用(Canvas2DComponent)とset用(TemporalComponent)で分けている。
 *
 */
class WEBCFACE_DLL Canvas2DComponent {
    std::shared_ptr<internal::Canvas2DComponentData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;
    int idx_for_type = 0;

    void checkData() const;

  public:
    /*!
     * msg_dataはnullptrになり、内容にアクセスしようとするとruntime_errorを投げる
     *
     */
    Canvas2DComponent();
    /*!
     * \param msg_data
     * \param data_w
     * \param idx_next 種類ごとの要素数のmap
     * InputRefの名前に使うidを決定するのに使う
     *
     */
    Canvas2DComponent(
        const std::shared_ptr<internal::Canvas2DComponentData> &msg_data,
        const std::weak_ptr<internal::ClientData> &data_w,
        std::unordered_map<Canvas2DComponentType, int> *idx_next);

    /*!
     * \brief そのcanvas2d内で一意のid
     * \since ver1.10
     *
     * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     *
     */
    std::string id() const;

    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas2DComponent &other) const;
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
    Canvas2DComponentType type() const;
    /*!
     * \brief 要素の移動・回転
     *
     */
    Transform origin() const;
    /*!
     * \brief 図形の輪郭の色
     *
     */
    ViewColor color() const;
    /*!
     * \brief 塗りつぶし色
     *
     */
    ViewColor fillColor() const;

    /*!
     * \brief 線の太さ
     *
     */
    double strokeWidth() const;
    /*!
     * \brief 文字の大きさ(高さ)
     * \since ver1.9
     */
    double textSize() const { return strokeWidth(); }
    /*!
     * \brief 表示する文字列
     * \since ver1.9
     */
    std::string text() const;
    /*!
     * \brief 表示する文字列 (wstring)
     * \since ver2.0
     */
    std::wstring textW() const;
    /*!
     * \brief geometryを取得
     *
     */
    std::optional<Geometry> geometry() const;
    /*!
     * \brief クリック時に実行される関数を取得
     * \since ver1.9
     */
    std::optional<Func> onClick() const;
};

class WEBCFACE_DLL TemporalCanvas2DComponent {
    std::unique_ptr<internal::Canvas2DComponentData> msg_data;

  public:
    /*!
     * msg_dataはnullptrになる
     *
     */
    explicit TemporalCanvas2DComponent(std::nullptr_t = nullptr);
    /*!
     * msg_dataを初期化する
     *
     */
    explicit TemporalCanvas2DComponent(ViewComponentType type);
    TemporalCanvas2DComponent(const TemporalCanvas2DComponent &other);
    TemporalCanvas2DComponent &
    operator=(const TemporalCanvas2DComponent &other);
    ~TemporalCanvas2DComponent() noexcept;

    /*!
     * \brief AnonymousFuncの名前を確定
     * \param data
     * \param view_name viewの名前
     * \param idx_next 種類ごとの要素数のmap
     * Funcの名前に使うidを決定するのに使う
     *
     */
    TemporalCanvas2DComponent &
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<Canvas2DComponentType, int> *idx_next = nullptr);

    /*!
     * \brief 要素の移動・回転
     *
     */
    TemporalCanvas2DComponent &origin(const Transform &origin);
    /*!
     * \brief 図形の輪郭の色
     *
     * 図形の輪郭の色を指定します。
     * デフォルト時のinheritはWebUI上ではblackとして表示されます
     *
     */
    TemporalCanvas2DComponent &color(const ViewColor &color);
    /*!
     * \brief 塗りつぶし色
     *
     * 図形の塗りつぶし色を指定します。
     * デフォルト時のinheritはWebUI上では透明になります
     *
     */
    TemporalCanvas2DComponent &fillColor(const ViewColor &color);
    /*!
     * \brief 線の太さ
     *
     * 図形の輪郭の太さを指定します。
     * 太さ1はCanvas2Dの座標系で1の長さ分の太さになります(拡大縮小で太さが変わる)
     *
     * 指定しない場合0となり、WebUIではその場合Canvasの拡大に関係なく1ピクセルになります
     *
     */
    TemporalCanvas2DComponent &strokeWidth(double s);
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
    TemporalCanvas2DComponent &textSize(double s) { return strokeWidth(s); }
    /*!
     * \brief 表示する文字列を設定
     * \since ver1.9
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    TemporalCanvas2DComponent &text(std::string_view text);
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    TemporalCanvas2DComponent &text(std::wstring_view text);
    /*!
     * \brief geometryをセット
     *
     */
    TemporalCanvas2DComponent &geometry(const Geometry &g);
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    TemporalCanvas2DComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func 実行する任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    TemporalCanvas2DComponent &onClick(T func) {
        return onClick(std::make_shared<AnonymousFunc>(func));
    }
    TemporalCanvas2DComponent &
    onClick(const std::shared_ptr<AnonymousFunc> &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (コピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    TemporalCanvas2DComponent &onClick(AnonymousFunc &&func) {
        return onClick(std::make_shared<AnonymousFunc>(std::move(func)));
    }
};

WEBCFACE_NS_END
