#pragma once
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/common/val_adaptor.h"
#include "webcface/text.h"

#ifdef min
// clang-format off
#pragma message("warning: Disabling macro definition of 'min' and 'max', since they conflicts in webcface/component_view.h.")
// clang-format on
#undef min
#undef max
#endif

extern "C" {
typedef struct wcfViewComponent wcfViewComponent;
typedef struct wcfViewComponentW wcfViewComponentW;
typedef struct wcfMultiVal wcfMultiVal;
typedef struct wcfMultiValW wcfMultiValW;
}
WEBCFACE_NS_BEGIN
namespace message {
struct ViewComponentData;
}
namespace internal {
struct TemporalViewComponentData;
struct ClientData;
class ViewBuf;
} // namespace internal
class Func;
class FuncListener;

enum class ViewColor {
    inherit = 0,
    black = 1,
    white = 2,
    // slate = 3,
    gray = 4,
    // zinc = 5,
    // neutral = 6,
    // stone = 7,
    red = 8,
    orange = 9,
    // amber = 10,
    yellow = 11,
    // lime = 12,
    green = 13,
    // emerald = 14,
    teal = 15,
    cyan = 16,
    // sky = 17,
    blue = 18,
    indigo = 19,
    // violet = 20,
    purple = 21,
    // fuchsia = 22,
    pink = 23,
    // rose = 24,
};
/*!
 * \brief ViewColorのenumの中からRGBで指定した色に近いものを返す
 * \since ver2.5
 *
 * * r, g, b はそれぞれ 0.0〜1.0 で指定
 * * 主観に基づいており、わりとてきとうです
 *
 */
WEBCFACE_DLL ViewColor colorFromRGB(double r, double g, double b);

enum class ViewComponentType {
    text = 0,
    new_line = 1,
    button = 2,
    text_input = 3,
    decimal_input = 4,
    number_input = 5,
    toggle_input = 6,
    select_input = 7,
    slider_input = 8,
    check_input = 9,
};

/*!
 * \brief Viewに表示する要素です
 *
 * * ver2.0〜: get専用(ViewComponent)とset用(TemporalComponent)で分けている。
 * * ver2.0〜: データはshared_ptrの中に持つ。(pimpl)
 *   * moveが多いしメンバ変数多いので、
 * make_sharedのコストはあまり気にしなくてもいい?
 * * 作成時
 *   * components::text() など →ViewComponent
 *   * v << move(component)
 *   * vb->addVC(move(component))
 *   * vb->add(move(component))
 *   * vb->components_.push_back(move(component))
 *   * setSend(make_shared(move(components_)))
 * * 取得時
 *   * getRecv()
 *   * vector<ViewComponent> v;
 *   * 1要素ずつ再構築
 *
 */
class WEBCFACE_DLL ViewComponent {
    std::shared_ptr<const message::ViewComponentData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;
    SharedString id_;

    // for cData()
    mutable std::unique_ptr<wcfMultiVal[]> options_s;
    mutable std::unique_ptr<wcfMultiValW[]> options_sw;

    void checkData() const;

    template <typename CComponent, typename CVal, std::size_t v_index>
    CComponent cDataT() const;

  public:
    /*!
     * msg_dataはnullptrになり、内容にアクセスしようとするとruntime_errorを投げる
     *
     */
    ViewComponent();

    ViewComponent(const std::shared_ptr<const message::ViewComponentData> &msg_data,
                  const std::weak_ptr<internal::ClientData> &data_w,
                  const SharedString &id);

    ViewComponent(const ViewComponent &);
    ViewComponent &operator=(const ViewComponent &);
    ViewComponent(ViewComponent &&) noexcept;
    ViewComponent &operator=(ViewComponent &&) noexcept;
    ~ViewComponent() noexcept;

    wcfViewComponent cData() const;
    wcfViewComponentW cDataW() const;

    /*!
     * \brief そのview内で一意のid
     * \since ver1.10
     *
     * * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     * * (ver2.5〜) view作成側でidを指定した場合その値が返る。
     *
     */
    std::string id() const;
    /*!
     * \brief そのview内で一意のid (wstring)
     * \since ver2.5
     *
     * * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     * * view作成側でidを指定した場合その値が返る。
     *
     */
    std::wstring idW() const;

    /*!
     * \brief 要素の比較
     * \since ver1.11
     *
     * 要素のプロパティが完全一致すればtrue
     *
     * 例外としてbindの中身は別のTextオブジェクトで管理されるので、
     * ここでは比較の対象ではない
     *
     */
    bool operator==(const ViewComponent &other) const;
    /*!
     * \since ver1.11
     */
    bool operator!=(const ViewComponent &other) const {
        return !(*this == other);
    }

    /*!
     * \brief 要素の種類
     *
     */
    ViewComponentType type() const;
    /*!
     * \brief 表示する文字列を取得
     *
     */
    std::string text() const;
    /*!
     * \brief 表示する文字列を取得 (wstring)
     * \since ver2.0
     */
    std::wstring textW() const;
    /*!
     * \brief クリック時に実行される関数を取得
     *
     */
    std::optional<Func> onClick() const;
    /*!
     * \brief inputの値の変化時に実行される関数を取得
     * \since ver1.10
     *
     * onChange()に新しい値を渡して呼び出すことで値を変更させる
     * (onChange()->runAsync(value) など)
     *
     * 内部データはonClickと共通
     *
     */
    std::optional<Func> onChange() const;
    /*!
     * \brief inputの現在の値を取得
     * \since ver1.10
     *
     * * 値の変更はonChange()に新しい値を渡して呼び出す
     * (onChange()->runAsync(value) など)
     * * ver2.0〜 戻り値をText型からVariant型に変更
     *
     */
    std::optional<Variant> bind() const;

    /*!
     * \brief 文字色を取得
     *
     */
    ViewColor textColor() const;
    /*!
     * \brief 背景色を取得
     *
     */
    ViewColor bgColor() const;
    /*!
     * \brief 最小値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> min() const;

    /*!
     * \brief 最大値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> max() const;
    /*!
     * \brief 数値の刻み幅を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> step() const;
    /*!
     * \brief 引数の選択肢を取得する。
     * \since ver1.10
     *
     */
    std::vector<ValAdaptor> option() const;
};

/*!
 * \brief Viewを構築するときに使う一時的なViewComponent
 * \since ver2.0
 *
 */
class WEBCFACE_DLL TemporalViewComponent {
    std::unique_ptr<internal::TemporalViewComponentData> msg_data;

  public:
    /*!
     * msg_dataはnullptrになる
     *
     */
    explicit TemporalViewComponent(std::nullptr_t = nullptr);
    /*!
     * msg_dataを初期化する
     *
     */
    explicit TemporalViewComponent(ViewComponentType type);
    TemporalViewComponent(const TemporalViewComponent &other);
    TemporalViewComponent &operator=(const TemporalViewComponent &other);
    /*!
     * \since ver2.5
     */
    TemporalViewComponent(TemporalViewComponent &&other) noexcept;
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &operator=(TemporalViewComponent &&other) noexcept;
    ~TemporalViewComponent() noexcept;

    friend class View;
    friend class internal::ViewBuf;

    /*!
     * \brief AnonymousFuncとInputRefの名前を確定
     * \param data
     * \param view_name viewの名前
     * \param idx_next 種類ごとの要素数のmap
     * InputRefの名前に使うidを決定するのに使う
     *
     */
    std::unique_ptr<internal::TemporalViewComponentData>
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<ViewComponentType, int> *idx_next = nullptr);

    /*!
     * \brief idを設定
     * \since ver2.5
     */
    TemporalViewComponent &id(std::string_view id);
    /*!
     * \brief idを設定 (wstring)
     * \since ver2.5
     */
    TemporalViewComponent &id(std::wstring_view id);
    /*!
     * \brief 表示する文字列を設定
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    TemporalViewComponent &text(std::string_view text) &;
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &&text(std::string_view text) && {
        this->text(text);
        return std::move(*this);
    }
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    TemporalViewComponent &text(std::wstring_view text) &;
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.5
     */
    TemporalViewComponent &&text(std::wstring_view text) && {
        this->text(text);
        return std::move(*this);
    }
    /*!
     * \brief クリック時に実行される関数を設定 (登録済みFunc)
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    TemporalViewComponent &onClick(const Func &func) &;
    /*!
     * \brief クリック時に実行される関数を設定 (登録済みFunc)
     * \since ver2.5
     */
    TemporalViewComponent &&onClick(const Func &func) && {
        this->onClick(func);
        return std::move(*this);
    }
    /*!
     * \brief クリック時に実行される関数を設定 (FuncListener)
     * \param func 実行する関数を指すFuncListener
     * \since ver2.5
     */
    TemporalViewComponent &onClick(const FuncListener &func) &;
    /*!
     * \brief クリック時に実行される関数を設定 (FuncListener)
     * \since ver2.5
     */
    TemporalViewComponent &&onClick(const FuncListener &func) && {
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
    TemporalViewComponent &onClick(T func) & {
        return onClick(std::make_shared<std::function<void WEBCFACE_CALL_FP()>>(
            std::move(func)));
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver2.5
     */
    template <typename T, decltype(std::declval<T>()(), nullptr) = nullptr>
    TemporalViewComponent &&onClick(T func) && {
        this->onClick(std::move(func));
        return std::move(*this);
    }
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &onClick(
        const std::shared_ptr<std::function<void WEBCFACE_CALL_FP()>> &func);

    /*!
     * \brief 変更した値を格納するInputRefを設定
     * \since ver1.10
     *
     * refの値を変更する処理が自動的にonChangeに登録される
     *
     */
    TemporalViewComponent &bind(const InputRef &ref) &;
    /*!
     * \brief 変更した値を格納するInputRefを設定
     * \since ver2.5
     */
    TemporalViewComponent &&bind(const InputRef &ref) && {
        this->bind(ref);
        return std::move(*this);
    }
    /*!
     * \brief 値が変化した時に実行される関数を設定
     * \since ver1.10
     *
     * 現在値を保持するInputRefは自動で生成されbindされる
     * \param func
     * 引数を1つ取る任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    TemporalViewComponent &onChange(T func) & {
        InputRef ref;
        return onChange(
            std::make_shared<std::function<void WEBCFACE_CALL_FP(ValAdaptor)>>(
                [ref, func = std::move(func)](ValAdaptor val) {
                    ref.lockedField().set(val);
                    return func(val);
                }),
            ref);
    }
    /*!
     * \brief 値が変化した時に実行される関数を設定
     * \since ver2.5
     */
    template <typename T>
    TemporalViewComponent &&onChange(T func) && {
        this->onChange(std::move(func));
        return std::move(*this);
    }
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &onChange(
        const std::shared_ptr<std::function<void WEBCFACE_CALL_FP(ValAdaptor)>>
            &func,
        const InputRef &ref);

    /*!
     * \brief 文字色を設定
     *
     */
    TemporalViewComponent &textColor(ViewColor c) &;
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &&textColor(ViewColor c) && {
        this->textColor(c);
        return std::move(*this);
    }
    /*!
     * \brief 背景色を設定
     *
     */
    TemporalViewComponent &bgColor(ViewColor c) &;
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &&bgColor(ViewColor c) && {
        this->bgColor(c);
        return std::move(*this);
    }
    /*!
     * \brief デフォルト値を設定する。
     * \since ver1.10
     *
     * デフォルト値はviewのメッセージには含まれるのではなく、
     * bindしたInputRefの初期化に使われる
     * (そのため component.init() では取得できない)
     *
     */
    template <typename T>
    TemporalViewComponent &init(const T &init) & {
        return this->init(ValAdaptor{init});
    }
    /*!
     * \brief デフォルト値を設定する。
     * \since ver2.5
     */
    template <typename T>
    TemporalViewComponent &&init(const T &init) && {
        this->init(init);
        return std::move(*this);
    }

  protected:
    TemporalViewComponent &init(const ValAdaptor &init);

  public:
    /*!
     * \brief 最小値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最小の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    TemporalViewComponent &min(double min) &;
    /*!
     * \brief 最小値を設定する。
     * \since ver2.5
     */
    TemporalViewComponent &&min(double min) && {
        this->min(min);
        return std::move(*this);
    }
    /*!
     * \brief 最大値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最大の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    TemporalViewComponent &max(double max) &;
    /*!
     * \brief 最大値を設定する。
     * \since ver2.5
     */
    TemporalViewComponent &&max(double max) && {
        this->max(max);
        return std::move(*this);
    }
    /*!
     * \brief 数値の刻み幅を設定する。
     * \since ver1.10
     *
     * * 整数入力、スライダーなど以外効果がない。
     *
     */
    TemporalViewComponent &step(double step) &;
    /*!
     * \brief 数値の刻み幅を設定する。
     * \since ver2.5
     */
    TemporalViewComponent &&step(double step) && {
        this->step(step);
        return std::move(*this);
    }

    /*!
     * \brief 引数の選択肢を設定する。
     * \since ver1.10
     *
     * * min(), max() はクリアされる。
     *
     */
    template <typename T>
    TemporalViewComponent &option(std::initializer_list<T> option) & {
        std::vector<ValAdaptor> option_v;
        for (const auto &v : option) {
            option_v.emplace_back(ValAdaptor(v));
        }
        return this->option(std::move(option_v));
    }
    /*!
     * \brief 引数の選択肢を設定する。
     * \since ver2.5
     */
    template <typename T>
    TemporalViewComponent &&option(std::initializer_list<T> option) && {
        this->option(option);
        return std::move(*this);
    }

    TemporalViewComponent &option(std::vector<ValAdaptor> option) &;
    /*!
     * \since ver2.5
     */
    TemporalViewComponent &&option(std::vector<ValAdaptor> option) && {
        this->option(std::move(option));
        return std::move(*this);
    }
};

WEBCFACE_NS_END
