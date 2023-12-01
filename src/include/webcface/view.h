#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <cassert>
#include <memory>
#include "common/view.h"
#include "func.h"
#include "event_target.h"

namespace webcface {
namespace Internal {
struct ClientData;
}
//! Viewに表示する要素です
class ViewComponent : protected Common::ViewComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

  public:
    ViewComponent() = default;
    ViewComponent(const Common::ViewComponentBase &vc,
                  const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::ViewComponentBase(vc), data_w(data_w) {}
    explicit ViewComponent(ViewComponentType type) { type_ = type; }

    //! AnonymousFuncをFuncオブジェクトにlockします
    WEBCFACE_DLL ViewComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
            const std::string &field_id);

    //! 要素の種類
    ViewComponentType type() const { return type_; }
    //! 表示する文字列を取得
    std::string text() const { return text_; }
    //! 表示する文字列を設定
    ViewComponent &text(const std::string &text) {
        text_ = text;
        return *this;
    }
    //! クリック時に実行される関数を取得
    WEBCFACE_DLL std::optional<Func> onClick() const;
    //! クリック時に実行される関数を設定
    WEBCFACE_DLL ViewComponent &onClick(const Func &func);
    //! クリック時に実行される関数を設定
    template <typename T>
    ViewComponent &onClick(const T &func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
    //! 文字色を取得
    ViewColor textColor() const { return text_color_; }
    //! 文字色を設定
    ViewComponent &textColor(ViewColor c) {
        text_color_ = c;
        return *this;
    }
    //! 背景色を取得
    ViewColor bgColor() const { return bg_color_; }
    //! 背景色を設定
    ViewComponent &bgColor(ViewColor c) {
        bg_color_ = c;
        return *this;
    }
};
inline namespace ViewComponents {
//! textコンポーネント
inline ViewComponent text(const std::string &text) {
    return ViewComponent(ViewComponentType::text).text(text);
}
//! newLineコンポーネント
inline ViewComponent newLine() {
    return ViewComponent(ViewComponentType::new_line);
}
//! buttonコンポーネント
template <typename T>
inline ViewComponent button(const std::string &text, const T &func) {
    return ViewComponent(ViewComponentType::button).text(text).onClick(func);
}
} // namespace ViewComponents

//! Viewの送信用データを保持する
class ViewBuf : public std::stringbuf {
  public:
    //! 送信用のデータ
    std::vector<ViewComponent> components;
    bool modified = false;
    //! std::flush時に呼ばれる
    WEBCFACE_DLL int sync() override;

    ViewBuf() : std::stringbuf(std::ios_base::out) {}
    ViewBuf(const ViewBuf &rhs) : ViewBuf() { *this = rhs; }
    ViewBuf &operator=(const ViewBuf &rhs) {
        this->components = rhs.components;
        this->modified = rhs.modified;
        return *this;
    }
};

//! Viewの送受信データを表すクラス
/*! コンストラクタではなく Member::view() を使って取得してください
 */
class View : protected Field, public EventTarget<View>, public std::ostream {
    ViewBuf sb;
    WEBCFACE_DLL void onAppend() const override;

    //! 値をセットし、EventTargetを発動する
    WEBCFACE_DLL View &set(std::vector<ViewComponent> &v);

  public:
    //! デフォルトコンストラクタ: clientと関連付けない
    /*!
     * 他のFieldをデフォルト構築した場合と同様、get()やset(), sync()などはruntime_errorを投げるが、
     * add()やoperator<<での要素の追加は可能で、
     * コンポーネントをグループ化して別のViewにコピーすることができる。
     */
    WEBCFACE_DLL View();
    WEBCFACE_DLL View(const Field &base);
    View(const Field &base, const std::string &field)
        : View(Field{base, field}) {}
    View(const View &rhs) : View() { *this = rhs; }
    WEBCFACE_DLL View &operator=(const View &rhs);

    WEBCFACE_DLL ~View();

    using Field::member;
    using Field::name;

    //! 子フィールドを返す
    /*!
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするView
     */
    View child(const std::string &field) {
        return View{*this, this->field_ + "." + field};
    }
    //! Viewを取得する
    WEBCFACE_DLL std::optional<std::vector<ViewComponent>> tryGet() const;
    //! Viewを取得する
    std::vector<ViewComponent> get() const {
        return tryGet().value_or(std::vector<ViewComponent>{});
    }
    //! syncの時刻を返す
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL View &free();

    //! このViewのViewBufの内容を初期化する
    /*!
     * * ver1.1まで: コンストラクタでも自動で呼ばれる。
     * * ver1.2以降:
     * このViewオブジェクトに追加された内容をクリアし、内容を変更済みとしてマークする
     * (init() 後に sync() をするとViewの内容が空になる)
     */
    WEBCFACE_DLL View &init();
    //! 文字列にフォーマットし、textコンポーネントとして追加
    /*!
     * std::ostream::operator<< でも同様の動作をするが、returnする型が異なる
     * (std::ostream & を返すと operator<<(ViewComponent) が使えなくなる)
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
    //! コンポーネントを追加
    /*!
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     */
    View &operator<<(const Common::ViewComponentBase &vc) {
        *this << ViewComponent{vc, this->data_w};
        return *this;
    }
    //! コンポーネントを追加
    /*!
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     */
    WEBCFACE_DLL View &operator<<(const ViewComponent &vc);
    //! 別のViewに含まれるコンポーネントを追加
    /*!
     * 対象のViewオブジェクトに直接追加されたコンポーネントがあればそれを追加、
     * なければ get() で取得したものを返す
     * (対象のViewがデフォルト構築でも例外は投げない)
     * 
     * std::flushも呼び出すことで直前に追加した未flashの文字列なども確実に追加する
     */
    WEBCFACE_DLL View &operator<<(const View &vg);

    //! コンポーネントなどを追加
    /*!
     * Tの型に応じた operator<< が呼ばれる
     */
    template <typename T>
    View &add(const T &rhs) {
        *this << rhs;
        return *this;
    }

    //! Viewの内容をclientに反映し送信可能にする
    /*!
     * デストラクタでも自動で呼ばれる。
     *
     * * ver1.2以降: このViewオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     */
    WEBCFACE_DLL View &sync();
};
} // namespace webcface
