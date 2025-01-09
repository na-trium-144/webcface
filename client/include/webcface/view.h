#pragma once
#include <vector>
#include <ostream>
#include <memory>
#include <utility>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "components.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename Component>
class DataSetBuffer;
class ViewBuf;
} // namespace internal

/*!
 * \brief Viewの送受信データを表すクラス
 *
 * コンストラクタではなく Member::view() を使って取得してください
 *
 */
class WEBCFACE_DLL View : protected Field {
    std::shared_ptr<internal::ViewBuf> sb;
    mutable std::ostream os;

    static constexpr std::nullptr_t TraitOk = nullptr;
    template <typename T>
    using EnableIfFormattable =
        decltype(std::declval<std::ostream>() << std::declval<T>(), TraitOk);
    template <typename T>
    using EnableIfInvocable =
        decltype(std::declval<T>()(std::declval<View>()), TraitOk);

  public:
    View();
    View(const Field &base);
    View(const Field &base, const SharedString &field)
        : View(Field{base, field}) {}
    View(const View &rhs) : View() { *this = rhs; }
    View(View &&rhs) noexcept : View() { *this = std::move(rhs); }
    View &operator=(const View &rhs);
    View &operator=(View &&rhs) noexcept;
    ~View();

    friend internal::DataSetBuffer<ViewComponent>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    View child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    View child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    View child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    View operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    View operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    View operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    View operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    View operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    View parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback View型の引数(thisが渡される)を1つ取る関数
     *
     */
    const View &
    onChange(std::function<void WEBCFACE_CALL_FP(View)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const View &onChange(F callback) const {
        return onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
    }
    /*!
     * \deprecated
     * ver1.11まではEventTarget::appendListener()でコールバックを追加できたが、
     * ver2.0からコールバックは1個のみになった。
     * 互換性のため残しているがonChange()と同じ
     *
     */
    template <typename T>
    [[deprecated]] void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief viewをリクエストする
     * \since ver1.7
     *
     */
    const View &request() const;
    /*!
     * \brief Viewを取得する
     *
     */
    std::optional<std::vector<ViewComponent>> tryGet() const;
    /*!
     * \brief Viewを取得する
     *
     */
    std::vector<ViewComponent> get() const {
        return tryGet().value_or(std::vector<ViewComponent>{});
    }
    /*!
     * \brief このフィールドにデータが存在すればtrue
     * \since ver2.1
     *
     * tryGet() などとは違って、実際のデータを受信しない。
     * リクエストも送信しない。
     *
     */
    bool exists() const;
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     *
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    const View &free() const;

    /*!
     * \brief このViewのViewBufの内容を初期化する
     *
     * * ver1.1まで: コンストラクタでも自動で呼ばれる。
     * * ver1.2以降:
     * このViewオブジェクトに追加された内容をクリアし、内容を変更済みとしてマークする
     * (init() 後に sync() をするとViewの内容が空になる)
     *
     */
    const View &init() const;
    /*!
     * \brief 文字列にフォーマットし、textコンポーネントとして追加
     *
     * std::ostream::operator<< でも同様の動作をするが、returnする型が異なる
     * (std::ostream & を返すと operator<<(ViewComponent) が使えなくなる)
     *
     * ver1.9〜 const参照ではなく&&型にしてforwardするようにした
     *
     */
    template <typename T, EnableIfFormattable<T> = TraitOk>
    const View &operator<<(T &&rhs) const {
        os << std::forward<T>(rhs);
        return *this;
    }
    const View &operator<<(std::ostream &(*os_manip)(std::ostream &)) const {
        os_manip(os);
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     *
     */
    template <bool C2, bool C3>
    const View &operator<<(TemporalComponent<true, C2, C3> vc) const {
        *this << std::move(vc.component_v);
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     *
     */
    const View &operator<<(TemporalViewComponent vc) const;
    /*!
     * \brief コンポーネントを追加
     * \since ver1.11
     *
     * カスタムコンポーネントとして引数にViewをとる関数を渡すことができる
     *
     */
    template <typename F, EnableIfInvocable<F> = TraitOk>
    const View &operator<<(const F &manip) const {
        manip(*this);
        return *this;
    }

    /*!
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     * ver1.9〜 const参照から&&に変更してforwardするようにした
     *
     */
    template <typename T>
    const View &add(T &&rhs) const {
        *this << std::forward<T>(rhs);
        return *this;
    }

    /*!
     * \brief back inserter iterator を返す
     * \since ver2.6
     *
     * * これが返すイテレーターを使うことでViewに文字列を追加できる。
     * * fmt::format_to や std::format_to に渡して使う
     *
     */
    std::ostreambuf_iterator<char> inserter() const {
        return std::ostreambuf_iterator<char>(os);
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * * ver1.2以降: このViewオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    const View &sync() const;

    /*!
     * \brief Viewの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, View>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, View>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
