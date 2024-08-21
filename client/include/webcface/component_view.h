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
namespace internal {
struct ViewComponentData;
class ViewBuf;
} // namespace internal

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
    std::shared_ptr<internal::ViewComponentData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;
    int idx_for_type = 0;

    void checkData() const;

  public:
    /*!
     * msg_dataはnullptrになり、内容にアクセスしようとするとruntime_errorを投げる
     *
     */
    ViewComponent();
    /*!
     * \param msg_data
     * \param data_w
     * \param idx_next 種類ごとの要素数のmap
     * InputRefの名前に使うidを決定するのに使う
     *
     */
    ViewComponent(const std::shared_ptr<internal::ViewComponentData> &msg_data,
                  const std::weak_ptr<internal::ClientData> &data_w,
                  std::unordered_map<ViewComponentType, int> *idx_next);

    wcfViewComponent cData() const;
    wcfViewComponentW cDataW() const;

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
    std::optional<Func> onChange() const { return onClick(); }
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
    std::unique_ptr<internal::ViewComponentData> msg_data;

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
    std::unique_ptr<internal::ViewComponentData>
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<ViewComponentType, int> *idx_next = nullptr);

    /*!
     * \brief 表示する文字列を設定
     *
     * (ver2.0からstring_viewに変更)
     *
     */
    TemporalViewComponent &text(std::string_view text);
    /*!
     * \brief 表示する文字列を設定 (wstring)
     * \since ver2.0
     */
    TemporalViewComponent &text(std::wstring_view text);

    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    TemporalViewComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    TemporalViewComponent &onClick(T func) {
        return onClick(std::make_shared<AnonymousFunc>(std::move(func)));
    }
    TemporalViewComponent &onClick(const std::shared_ptr<AnonymousFunc> &func);

    /*!
     * \brief クリック時に実行される関数を設定
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (ver1.9からコピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    TemporalViewComponent &onClick(AnonymousFunc &&func) {
        return onClick(std::make_shared<AnonymousFunc>(std::move(func)));
    }

    /*!
     * \brief 変更した値を格納するInputRefを設定
     * \since ver1.10
     *
     * refの値を変更する処理が自動的にonChangeに登録される
     *
     */
    TemporalViewComponent &bind(const InputRef &ref);

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
    TemporalViewComponent &onChange(T func) {
        InputRef ref;
        return onChange(
            std::make_shared<AnonymousFunc>([ref, func](ValAdaptor val) {
                ref.lockedField().set(val);
                return func(val);
            }),
            ref);
    }
    TemporalViewComponent &onChange(const std::shared_ptr<AnonymousFunc> &func,
                                    const InputRef &ref);
    // /*!
    //  * \brief 値が変化した時に実行される関数を設定
    //  * \since ver1.10
    //  *
    //  */
    // TemporalViewComponent &onChange(AnonymousFunc &&func) {
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
     * \brief 文字色を設定
     *
     */
    TemporalViewComponent &textColor(ViewColor c);
    /*!
     * \brief 背景色を設定
     *
     */
    TemporalViewComponent &bgColor(ViewColor c);
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
    TemporalViewComponent &init(const T &init) {
        return this->init(ValAdaptor{init});
    }
    TemporalViewComponent &init(const ValAdaptor &init);

    /*!
     * \brief 最小値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最小の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    TemporalViewComponent &min(double min);
    /*!
     * \brief 最大値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最大の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    TemporalViewComponent &max(double max);
    /*!
     * \brief 数値の刻み幅を設定する。
     * \since ver1.10
     *
     * * 整数入力、スライダーなど以外効果がない。
     *
     */
    TemporalViewComponent &step(double step);

    /*!
     * \brief 引数の選択肢を設定する。
     * \since ver1.10
     *
     * * min(), max() はクリアされる。
     *
     */
    template <typename T>
    TemporalViewComponent &option(std::initializer_list<T> option) {
        std::vector<ValAdaptor> option_v;
        for (const auto &v : option) {
            option_v.emplace_back(ValAdaptor(v));
        }
        return this->option(std::move(option_v));
    }
    TemporalViewComponent &option(std::vector<ValAdaptor> option);
};

WEBCFACE_NS_END
