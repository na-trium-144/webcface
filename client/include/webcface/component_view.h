#pragma once
#include <optional>
#include <vector>
#include "field.h"
#include <webcface/common/def.h>
#include "webcface/encoding/val_adaptor.h"
#include "component_id.h"
#include "webcface/func.h"
#include "webcface/text.h"

#ifdef min
// clang-format off
#pragma message("warning: Disabling macro definition of 'min' and 'max', since they conflicts in webcface/component_view.h.")
// clang-format on
#undef min
#undef max
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct ViewComponent;
}

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

#if WEBCFACE_SYSTEM_DLLEXPORT
extern template class WEBCFACE_DLL_INSTANCE_DECL IdBase<ViewComponentType>;
#endif

/*!
 * \brief Viewに表示する要素です
 *
 */
class WEBCFACE_DLL ViewComponent : public IdBase<ViewComponentType> {
    std::weak_ptr<internal::ClientData> data_w;

    ViewComponentType type_ = ViewComponentType::text;
    SharedString text_;
    std::optional<FieldBase> on_click_func_;
    std::optional<FieldBase> text_ref_;
    ViewColor text_color_ = ViewColor::inherit;
    ViewColor bg_color_ = ViewColor::inherit;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt,
                          step_ = std::nullopt;
    std::vector<ValAdaptor> option_ = {};

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;

    // for cData()
    mutable std::variant<std::vector<wcfMultiVal>, std::vector<wcfMultiValW>>
        options_s;

    template <typename CComponent, typename CVal, std::size_t v_index>
    CComponent cDataT() const;

  public:
    ViewComponent() = default;
    ViewComponent(ViewComponentType type, const SharedString &text,
                  std::optional<FieldBase> &&on_click_func,
                  std::optional<FieldBase> &&text_ref, ViewColor text_color,
                  ViewColor bg_color, std::optional<double> min,
                  std::optional<double> max, std::optional<double> step,
                  std::vector<ValAdaptor> &&option)
        : type_(type), text_(text), on_click_func_(std::move(on_click_func)),
          text_ref_(std::move(text_ref)), text_color_(text_color),
          bg_color_(bg_color), min_(min), max_(max), step_(step),
          option_(std::move(option)) {}
    ViewComponent(const ViewComponent &vc,
                  const std::weak_ptr<internal::ClientData> &data_w,
                  std::unordered_map<int, int> *idx_next)
        : ViewComponent(vc) {
        this->data_w = data_w;
        initIdx(idx_next, type_);
    }
    explicit ViewComponent(ViewComponentType type)
        : IdBase<ViewComponentType>() {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncとInputRefの名前を確定
     *
     */
    ViewComponent &lockTmp(const std::weak_ptr<internal::ClientData> &data_w,
                           const SharedString &view_name,
                           std::unordered_map<int, int> *idx_next = nullptr);

    wcfViewComponent cData() const;
    wcfViewComponentW cDataW() const;
    message::ViewComponent toMessage() const;
    ViewComponent(const message::ViewComponent &vc);

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
    bool operator==(const ViewComponent &other) const {
        return id() == other.id() && type_ == other.type_ &&
               text_ == other.text_ && on_click_func_ == other.on_click_func_ &&
               text_ref_ == other.text_ref_ &&
               text_color_ == other.text_color_ &&
               bg_color_ == other.bg_color_ && min_ == other.min_ &&
               max_ == other.max_ && step_ == other.step_ &&
               option_ == other.option_;
    }
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
    ViewComponentType type() const override { return type_; }
    /*!
     * \brief 表示する文字列を取得
     *
     */
    std::string text() const { return text_.decode(); }
    /*!
     * \brief 表示する文字列を取得 (wstring)
     * \since ver2.0
     */
    std::wstring textW() const { return text_.decodeW(); }
    /*!
     * \brief 表示する文字列を設定
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    ViewComponent &text(std::string_view text) {
        text_ = SharedString::encode(text);
        return *this;
    }
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    ViewComponent &text(std::wstring_view text) {
        text_ = SharedString::encode(text);
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を取得
     *
     */
    std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    ViewComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    ViewComponent &onClick(T func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (ver1.9からコピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    ViewComponent &onClick(AnonymousFunc &&func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(std::move(func));
        return *this;
    }
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
    std::optional<Func> onChange() const { return onClick(); }
    /*!
     * \brief 変更した値を格納するInputRefを設定
     * \since ver1.10
     *
     * refの値を変更する処理が自動的にonChangeに登録される
     *
     */
    ViewComponent &bind(const InputRef &ref) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(
            [ref](const ValAdaptor &val) { ref.lockedField().set(val); });
        text_ref_tmp = ref;
        return *this;
    }
    /*!
     * \brief inputの現在の値を取得
     * \since ver1.10
     *
     *     * 値の変更はonChange()に新しい値を渡して呼び出す
     * (onChange()->runAsync(value) など)
     *
     */
    std::optional<Text> bind() const;
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
    ViewComponent &onChange(T func) {
        InputRef ref;
        on_click_func_tmp =
            std::make_shared<AnonymousFunc>([ref, func](ValAdaptor val) {
                ref.lockedField().set(val);
                return func(val);
            });
        text_ref_tmp = ref;
        return *this;
    }
    // /*!
    //  * \brief 値が変化した時に実行される関数を設定
    //  * \since ver1.10
    //  *
    //  */
    // ViewComponent &onChange(AnonymousFunc &&func) {
    //     InputRef ref;
    //     auto func_impl = func.getImpl();
    //     func.replaceImpl([ref, func_impl](const std::vector<ValAdaptor>
    //     &args) {
    //         if (args.size() >= 1) {
    //             ref.lockedField().set(args[0]);
    //         }
    //         return func_impl(args);
    //     });
    //     on_click_func_tmp = std::make_shared<AnonymousFunc>(std::move(func));
    //     text_ref_tmp = ref;
    //     return *this;
    // }
    /*!
     * \brief 文字色を取得
     *
     */
    ViewColor textColor() const { return text_color_; }
    /*!
     * \brief 文字色を設定
     *
     */
    ViewComponent &textColor(ViewColor c) {
        text_color_ = c;
        return *this;
    }
    /*!
     * \brief 背景色を取得
     *
     */
    ViewColor bgColor() const { return bg_color_; }
    /*!
     * \brief 背景色を設定
     *
     */
    ViewComponent &bgColor(ViewColor c) {
        bg_color_ = c;
        return *this;
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
    ViewComponent &init(const T &init) {
        init_.emplace(ValAdaptor(init));
        return *this;
    }
    /*!
     * \brief 最小値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> min() const { return min_; }
    /*!
     * \brief 最小値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最小の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    ViewComponent &min(double min) {
        min_ = min;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 最大値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> max() const { return max_; }
    /*!
     * \brief 最大値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最大の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    ViewComponent &max(double max) {
        max_ = max;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 数値の刻み幅を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> step() const { return step_; }
    /*!
     * \brief 数値の刻み幅を設定する。
     * \since ver1.10
     *
     * * 整数入力、スライダーなど以外効果がない。
     *
     */
    ViewComponent &step(double step) {
        step_ = step;
        return *this;
    }
    /*!
     * \brief 引数の選択肢を取得する。
     * \since ver1.10
     *
     */
    std::vector<ValAdaptor> option() const { return option_; }
    ViewComponent &option(std::vector<ValAdaptor> option) {
        option_ = std::move(option);
        min_ = max_ = std::nullopt;
        return *this;
    }
    /*!
     * \brief 引数の選択肢を設定する。
     * \since ver1.10
     *
     * * min(), max() はクリアされる。
     *
     */
    template <typename T>
    ViewComponent &option(std::initializer_list<T> option) {
        std::vector<ValAdaptor> option_v;
        for (const auto &v : option) {
            option_v.emplace_back(ValAdaptor(v));
        }
        return this->option(std::move(option_v));
    }
};


WEBCFACE_NS_END
