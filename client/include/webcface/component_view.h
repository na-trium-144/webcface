#pragma once
#include <optional>
#include <vector>
#include "field.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
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
namespace internal {
struct ViewComponentData;
}

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
 * \brief Viewに表示する要素です
 *
 * * ver2.0〜: データはunique_ptrの中に持つ。(pimpl)
 *   * moveが多いしメンバ変数多いので、
 * make_uniqueのコストはあまり気にしなくてもいい?
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
    std::unique_ptr<internal::ViewComponentData> data;

    void checkData() const;

  public:
    ViewComponent();
    ViewComponent(ViewComponentType type, const SharedString &text,
                  std::optional<FieldBase> &&on_click_func,
                  std::optional<FieldBase> &&text_ref, ViewColor text_color,
                  ViewColor bg_color, std::optional<double> min,
                  std::optional<double> max, std::optional<double> step,
                  std::vector<ValAdaptor> &&option);
    ViewComponent(const ViewComponent &vc,
                  const std::weak_ptr<internal::ClientData> &data_w,
                  std::unordered_map<int, int> *idx_next);
    explicit ViewComponent(ViewComponentType type);
    ViewComponent(const ViewComponent &other);
    ViewComponent &operator=(const ViewComponent &other);
    ~ViewComponent() noexcept;

    /*!
     * \brief AnonymousFuncとInputRefの名前を確定
     *
     */
    ViewComponent &lockTmp(const std::shared_ptr<internal::ClientData> &data,
                           const SharedString &view_name,
                           std::unordered_map<int, int> *idx_next = nullptr);

    wcfViewComponent cData() const;
    wcfViewComponentW cDataW() const;
    message::ViewComponent toMessage() const;
    ViewComponent(const message::ViewComponent &vc);

    /*!
     * \brief そのview内で一意のid
     * \since ver1.10
     *
     * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     *
     */
    std::string id() const;

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
     * \brief 表示する文字列を設定
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    ViewComponent &text(std::string_view text);
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    ViewComponent &text(std::wstring_view text);
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
        return onClick(std::make_shared<AnonymousFunc>(std::move(func)));
    }
    ViewComponent &onClick(const std::shared_ptr<AnonymousFunc> &func);

    /*!
     * \brief クリック時に実行される関数を設定
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (ver1.9からコピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    ViewComponent &onClick(AnonymousFunc &&func) {
        return onClick(std::make_shared<AnonymousFunc>(std::move(func)));
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
    ViewComponent &bind(const InputRef &ref);
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
        return onChange(
            std::make_shared<AnonymousFunc>([ref, func](ValAdaptor val) {
                ref.lockedField().set(val);
                return func(val);
            }),
            ref);
    }
    ViewComponent &onChange(const std::shared_ptr<AnonymousFunc> &func,
                            const InputRef &ref);
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
    ViewColor textColor() const;
    /*!
     * \brief 文字色を設定
     *
     */
    ViewComponent &textColor(ViewColor c);
    /*!
     * \brief 背景色を取得
     *
     */
    ViewColor bgColor() const;
    /*!
     * \brief 背景色を設定
     *
     */
    ViewComponent &bgColor(ViewColor c);
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
        return this->init(ValAdaptor{init});
    }
    ViewComponent &init(const ValAdaptor &init);
    /*!
     * \brief 最小値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> min() const;
    /*!
     * \brief 最小値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最小の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    ViewComponent &min(double min);
    /*!
     * \brief 最大値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> max() const;
    /*!
     * \brief 最大値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最大の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    ViewComponent &max(double max);
    /*!
     * \brief 数値の刻み幅を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> step() const;
    /*!
     * \brief 数値の刻み幅を設定する。
     * \since ver1.10
     *
     * * 整数入力、スライダーなど以外効果がない。
     *
     */
    ViewComponent &step(double step);
    /*!
     * \brief 引数の選択肢を取得する。
     * \since ver1.10
     *
     */
    std::vector<ValAdaptor> option() const;
    ViewComponent &option(std::vector<ValAdaptor> option);
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
