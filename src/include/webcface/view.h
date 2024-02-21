#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include "common/view.h"
#include "func.h"
#include "event_target.h"
#include "common/def.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
/*!
 * \brief Viewに表示する要素です
 *
 */
class ViewComponent : protected Common::ViewComponentBase {
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
    WEBCFACE_DLL ViewComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
            const std::string &field_id);

    /*!
     * \brief 要素の種類
     *
     */
    ViewComponentType type() const { return type_; }
    /*!
     * \brief 表示する文字列を取得
     *
     */
    std::string text() const { return text_; }
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
    WEBCFACE_DLL std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     *
     */
    WEBCFACE_DLL ViewComponent &onClick(const Func &func);
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

/*!
 * \brief Viewの送信用データを保持する
 *
 */
class ViewBuf : public std::stringbuf {
  public:
    /*!
     * \brief 送信用のデータ
     *
     */
    std::vector<ViewComponent> components;
    bool modified = false;

    WEBCFACE_DLL int sync() override;

    ViewBuf() : std::stringbuf(std::ios_base::out) {}
    ViewBuf(const ViewBuf &) = delete;
    ViewBuf &operator=(const ViewBuf &) = delete;
};

/*!
 * \brief Viewの送受信データを表すクラス
 *
 * コンストラクタではなく Member::view() を使って取得してください
 *
 */
class View : protected Field, public EventTarget<View>, public std::ostream {
    std::shared_ptr<ViewBuf> sb;

    WEBCFACE_DLL void onAppend() const override;

    /*!
     * \brief 値をセットし、EventTargetを発動する
     *
     */
    WEBCFACE_DLL View &set(std::vector<ViewComponent> &v);

    WEBCFACE_DLL void onDestroy();

  public:
    WEBCFACE_DLL View();
    WEBCFACE_DLL View(const Field &base);
    View(const Field &base, const std::string &field)
        : View(Field{base, field}) {}
    View(const View &rhs) : View() { *this = rhs; }
    WEBCFACE_DLL View &operator=(const View &rhs);
    WEBCFACE_DLL View &operator=(View &&rhs);

    /*!
     * \brief デストラクタで sync() を呼ぶ。
     *
     * * ver1.2以降:
     * Viewをコピーした場合は、すべてのコピーが破棄されたときにのみ sync()
     * が呼ばれる。
     * \sa sync()
     *
     */
    ~View() override { onDestroy(); }

    using Field::member;
    using Field::name;

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
    WEBCFACE_DLL void request() const;
    /*!
     * \brief Viewを取得する
     *
     */
    WEBCFACE_DLL std::optional<std::vector<ViewComponent>> tryGet() const;
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
    [[deprecated]] WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    WEBCFACE_DLL View &free();

    /*!
     * \brief このViewのViewBufの内容を初期化する
     *
     * * ver1.1まで: コンストラクタでも自動で呼ばれる。
     * * ver1.2以降:
     * このViewオブジェクトに追加された内容をクリアし、内容を変更済みとしてマークする
     * (init() 後に sync() をするとViewの内容が空になる)
     *
     */
    WEBCFACE_DLL View &init();
    /*!
     * \brief 文字列にフォーマットし、textコンポーネントとして追加
     *
     * std::ostream::operator<< でも同様の動作をするが、returnする型が異なる
     * (std::ostream & を返すと operator<<(ViewComponent) が使えなくなる)
     *
     */
    template <typename T>
    View &operator<<(const T &rhs) {
        static_cast<std::ostream &>(*this) << rhs;
        return *this;
    }
    View &operator<<(std::ostream &(*os_manip)(std::ostream &)) {
        os_manip(*this);
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     *
     */
    View &operator<<(const Common::ViewComponentBase &vc) {
        *this << ViewComponent{vc, this->data_w};
        return *this;
    }
    /*!
     * \brief コンポーネントを追加
     *
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     *
     */
    WEBCFACE_DLL View &operator<<(const ViewComponent &vc);

    /*!
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     */
    template <typename T>
    View &add(const T &rhs) {
        *this << rhs;
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * * ver1.2以降: このViewオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    WEBCFACE_DLL View &sync();
};
} // namespace WEBCFACE_NS
