#pragma once
#include <vector>
#include <ostream>
#include <memory>
#include <utility>
#include "common/view.h"
#include "event_target.h"
#include <webcface/common/def.h>
#include "canvas_data.h"

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename Component>
class DataSetBuffer;
class ViewBuf;
} // namespace Internal

/*!
 * \brief Viewの送受信データを表すクラス
 *
 * コンストラクタではなく Member::view() を使って取得してください
 *
 */
class WEBCFACE_DLL View : protected Field,
                          public EventTarget<View>,
                          public std::ostream {
    std::shared_ptr<Internal::ViewBuf> sb;

    void onAppend() const override final;

  public:
    View();
    View(const Field &base);
    View(const Field &base, std::u8string_view field)
        : View(Field{base, field}) {}
    View(const View &rhs) : View() { *this = rhs; }
    View(View &&rhs) noexcept : View() { *this = std::move(rhs); }
    View &operator=(const View &rhs);
    View &operator=(View &&rhs) noexcept;
    ~View() override;

    friend Internal::DataSetBuffer<ViewComponent>;

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
     * \since ver1.12
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
     * \since ver1.12
     */
    View operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    View operator[](const char *field) const { return child(field); }
    /*!
     * \since ver1.12
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
     * \brief viewをリクエストする
     * \since ver1.7
     *
     */
    void request() const;
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
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     *
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    View &free();

    /*!
     * \brief このViewのViewBufの内容を初期化する
     *
     * * ver1.1まで: コンストラクタでも自動で呼ばれる。
     * * ver1.2以降:
     * このViewオブジェクトに追加された内容をクリアし、内容を変更済みとしてマークする
     * (init() 後に sync() をするとViewの内容が空になる)
     *
     */
    View &init();
    /*!
     * \brief 文字列にフォーマットし、textコンポーネントとして追加
     *
     * std::ostream::operator<< でも同様の動作をするが、returnする型が異なる
     * (std::ostream & を返すと operator<<(ViewComponent) が使えなくなる)
     *
     * ver1.9〜 const参照ではなく&&型にしてforwardするようにした
     *
     */
    template <typename T>
        requires requires(T rhs) { std::ostream(nullptr) << rhs; }
    View &operator<<(T &&rhs) {
        static_cast<std::ostream &>(*this) << std::forward<T>(rhs);
        return *this;
    }
    View &operator<<(std::ostream &(*os_manip)(std::ostream &)) {
        os_manip(*this);
        return *this;
    }
    View &operator<<(Common::ViewComponentBase &vc) {
        *this << ViewComponent{vc, this->data_w, 0};
        return *this;
    }
    View &operator<<(const Common::ViewComponentBase &vc) {
        *this << ViewComponent{vc, this->data_w, 0};
        return *this;
    }
    View &operator<<(Common::ViewComponentBase &&vc) {
        *this << ViewComponent{vc, this->data_w, 0};
        return *this;
    }
    template <bool C2, bool C3>
    View &operator<<(TemporalComponent<true, C2, C3> &vc) {
        *this << vc.toV();
        return *this;
    }
    template <bool C2, bool C3>
    View &operator<<(const TemporalComponent<true, C2, C3> &vc) {
        *this << vc.toV();
        return *this;
    }
    template <bool C2, bool C3>
    View &operator<<(TemporalComponent<true, C2, C3> &&vc) {
        *this << std::move(vc.toV());
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     *
     */
    View &operator<<(const ViewComponent &vc);
    View &operator<<(ViewComponent &vc) {
        *this << static_cast<const ViewComponent &>(vc);
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     * \since ver1.9
     *
     */
    View &operator<<(ViewComponent &&vc);
    /*!
     * \brief コンポーネントを追加
     * \since ver1.11
     *
     * カスタムコンポーネントとして引数にViewをとる関数を渡すことができる
     *
     */
    template <typename F>
        requires std::invocable<F, View &>
    View &operator<<(const F &manip) {
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
    View &add(T &&rhs) {
        *this << std::forward<T>(rhs);
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * * ver1.2以降: このViewオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    View &sync();

    /*!
     * \brief Viewの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, View>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
