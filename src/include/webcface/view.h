#pragma once
#include <vector>
#include <ostream>
#include <memory>
#include <utility>
#include "common/view.h"
#include "func.h"
#include "event_target.h"
#include "common/def.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
template <typename Component>
class DataSetBuffer;
class ViewBuf;
}
/*!
 * \brief Viewに表示する要素です
 *
 */
class WEBCFACE_DLL ViewComponent : protected Common::ViewComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

  public:
    ViewComponent() = default;
    ViewComponent(const Common::ViewComponentBase &vc,
                  const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::ViewComponentBase(vc), data_w(data_w) {}
    explicit ViewComponent(ViewComponentType type) { type_ = type; }

    /*!
     * \brief AnonymousFuncをFuncオブジェクトにlockします
     *
     */
    ViewComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
            const std::string &field_id);

    wcfViewComponent cData() const;

    /*!
     * \brief 要素の種類
     *
     */
    ViewComponentType type() const { return type_; }
    /*!
     * \brief 表示する文字列を取得
     *
     */
    const std::string &text() const { return text_; }
    /*!
     * \brief 表示する文字列を設定
     *
     */
    ViewComponent &text(const std::string &text) {
        text_ = text;
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を取得
     *
     */
    std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     *
     */
    ViewComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     *
     */
    template <typename T>
    ViewComponent &onClick(const T &func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
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
};
inline namespace ViewComponents {
/*!
 * \brief textコンポーネント
 *
 */
inline ViewComponent text(const std::string &text) {
    return ViewComponent(ViewComponentType::text).text(text);
}
/*!
 * \brief newLineコンポーネント
 *
 */
inline ViewComponent newLine() {
    return ViewComponent(ViewComponentType::new_line);
}
/*!
 * \brief buttonコンポーネント
 *
 */
template <typename T>
inline ViewComponent button(const std::string &text, const T &func) {
    return ViewComponent(ViewComponentType::button).text(text).onClick(func);
}
} // namespace ViewComponents


class View;
extern template class WEBCFACE_IMPORT EventTarget<View>;

/*!
 * \brief Viewの送受信データを表すクラス
 *
 * コンストラクタではなく Member::view() を使って取得してください
 *
 */
class WEBCFACE_DLL View : protected Field, public EventTarget<View>, public std::ostream {
    std::shared_ptr<Internal::ViewBuf> sb;

    void onAppend() const override;

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
    [[deprecated]] std::chrono::system_clock::time_point
    time() const;

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
        *this << ViewComponent{vc, this->data_w};
        return *this;
    }
    View &operator<<(Common::ViewComponentBase &&vc) {
        *this << ViewComponent{vc, this->data_w};
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     *
     */
    View &operator<<(ViewComponent &vc);
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
} // namespace WEBCFACE_NS
