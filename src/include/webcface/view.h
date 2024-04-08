#pragma once
#include <vector>
#include <ostream>
#include <memory>
#include <utility>
#include "common/view.h"
#include "event_target.h"
#include "common/def.h"
#include "canvas_data.h"

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
template <typename Component>
class DataSetBuffer;
class ViewBuf;
} // namespace Internal

class View;
extern template class WEBCFACE_IMPORT EventTarget<View>;

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
    View(const Field &base, const std::string &field)
        : View(Field{base, field}) {}
    View(const View &rhs) : View() { *this = rhs; }
    View(View &&rhs) : View() { *this = std::move(rhs); }
    View &operator=(const View &rhs);
    View &operator=(View &&rhs);
    ~View() override;

    using Field::member;
    using Field::name;

    friend Internal::DataSetBuffer<ViewComponent>;

    /*!
     * \brief 子フィールドを返す
     *
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするView
     *
     */
    View child(const std::string &field) const {
        return View{*this, this->field_ + "." + field};
    }
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
};
WEBCFACE_NS_END
