#pragma once
#include <unordered_map>
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
struct Canvas2DComponentData;
}
namespace internal {
struct TemporalCanvas2DComponentData;
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
    std::shared_ptr<message::Canvas2DComponentData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;
    SharedString id_;

    void checkData() const;

  public:
    /*!
     * msg_dataはnullptrになり、内容にアクセスしようとするとruntime_errorを投げる
     *
     */
    Canvas2DComponent();

    Canvas2DComponent(
        const std::shared_ptr<message::Canvas2DComponentData> &msg_data,
        const std::weak_ptr<internal::ClientData> &data_w,
        const SharedString &id);

    /*!
     * \brief そのcanvas2d内で一意のid
     * \since ver1.10
     *
     * * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     * * (ver2.5〜) canvas2d作成側でidを指定した場合その値が返る。
     *
     */
    std::string id() const;
    /*!
     * \brief そのcanvas2d内で一意のid (wstring)
     * \since ver2.5
     *
     * * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     * * canvas2d作成側でidを指定した場合その値が返る。
     *
     */
    std::wstring idW() const;

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
    std::unique_ptr<internal::TemporalCanvas2DComponentData> msg_data;

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
    explicit TemporalCanvas2DComponent(Canvas2DComponentType type);
    TemporalCanvas2DComponent(const TemporalCanvas2DComponent &other);
    TemporalCanvas2DComponent &
    operator=(const TemporalCanvas2DComponent &other);
    TemporalCanvas2DComponent(TemporalCanvas2DComponent &&other) noexcept;
    TemporalCanvas2DComponent &
    operator=(TemporalCanvas2DComponent &&other) noexcept;
    ~TemporalCanvas2DComponent() noexcept;

    /*!
     * \brief AnonymousFuncの名前を確定
     * \param data
     * \param view_name viewの名前
     * \param idx_next 種類ごとの要素数のmap
     * Funcの名前に使うidを決定するのに使う
     *
     */
    std::unique_ptr<internal::TemporalCanvas2DComponentData>
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<Canvas2DComponentType, int> *idx_next = nullptr);

    /*!
     * \brief idを設定
     * \since ver2.5
     */
    TemporalCanvas2DComponent &id(std::string_view id);
    /*!
     * \brief idを設定 (wstring)
     * \since ver2.5
     */
    TemporalCanvas2DComponent &id(std::wstring_view id);
    /*!
     * \brief 要素の移動・回転
     *
     */
    TemporalCanvas2DComponent &origin(const Transform &origin) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&origin(const Transform &origin) && {
        this->origin(origin);
        return std::move(*this);
    }
    /*!
     * \brief 図形の輪郭の色
     *
     * 図形の輪郭の色を指定します。
     * デフォルト時のinheritはWebUI上ではblackとして表示されます
     *
     */
    TemporalCanvas2DComponent &color(const ViewColor &color) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&color(const ViewColor &color) && {
        this->color(color);
        return std::move(*this);
    }
    /*!
     * \brief 塗りつぶし色
     *
     * 図形の塗りつぶし色を指定します。
     * デフォルト時のinheritはWebUI上では透明になります
     *
     */
    TemporalCanvas2DComponent &fillColor(const ViewColor &color) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&fillColor(const ViewColor &color) && {
        this->fillColor(color);
        return std::move(*this);
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
    TemporalCanvas2DComponent &strokeWidth(double s) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&strokeWidth(double s) && {
        this->strokeWidth(s);
        return std::move(*this);
    }
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
    TemporalCanvas2DComponent &textSize(double s) & { return strokeWidth(s); }
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&textSize(double s) && {
        this->textSize(s);
        return std::move(*this);
    }
    /*!
     * \brief 表示する文字列を設定
     * \since ver1.9
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    TemporalCanvas2DComponent &text(std::string_view text) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&text(std::string_view text) && {
        this->text(text);
        return std::move(*this);
    }
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    TemporalCanvas2DComponent &text(std::wstring_view text) &;
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&text(std::wstring_view text) && {
        this->text(text);
        return std::move(*this);
    }
    /*!
     * \brief geometryをセット
     *
     */
    TemporalCanvas2DComponent &geometry(const Geometry &g) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&geometry(const Geometry &g) && {
        this->geometry(g);
        return std::move(*this);
    }
    /*!
     * \brief クリック時に実行される関数を設定 (Funcオブジェクト)
     * \since ver1.9
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    TemporalCanvas2DComponent &onClick(const Func &func) &;
    /*!
     * \brief クリック時に実行される関数を設定 (Funcオブジェクト)
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&onClick(const Func &func) && {
        this->onClick(func);
        return std::move(*this);
    }
    /*!
     * \brief クリック時に実行される関数を設定 (FuncListener)
     * \param func 実行する関数を指すFuncListener
     * \since ver2.5
     */
    TemporalCanvas2DComponent &onClick(const FuncListener &func) &;
    /*!
     * \brief クリック時に実行される関数を設定 (FuncListener)
     * \since ver2.5
     */
    TemporalCanvas2DComponent &&onClick(const FuncListener &func) && {
        this->onClick(func);
        return std::move(*this);
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する任意の関数
     * (引数、戻り値なしでstd::functionにキャスト可能ならなんでもok)
     * \since ver2.5
     *
     * MSVCのバグでエラーになってしまうので std::is_invocable_v は使えない
     *
     */
    template <typename T, decltype(std::declval<T>()(), nullptr) = nullptr>
    TemporalCanvas2DComponent &onClick(T func) & {
        return onClick(std::make_shared<std::function<void WEBCFACE_CALL_FP()>>(
            std::move(func)));
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver2.5
     */
    template <typename T, decltype(std::declval<T>()(), nullptr) = nullptr>
    TemporalCanvas2DComponent &&onClick(T func) && {
        this->onClick(std::move(func));
        return std::move(*this);
    }

    TemporalCanvas2DComponent &onClick(
        const std::shared_ptr<std::function<void WEBCFACE_CALL_FP()>> &func);
};

WEBCFACE_NS_END
