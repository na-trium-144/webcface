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
    //! std::flush時に呼ばれる
    int sync() override {
        std::string s = this->str();
        while (true) {
            auto p = s.find('\n');
            if (p == std::string::npos) {
                break;
            }
            std::string c1 = s.substr(0, p);
            if (!c1.empty()) {
                components.push_back(ViewComponents::text(c1));
            }
            components.push_back(ViewComponents::newLine());
            s = s.substr(p + 1);
        }
        if (!s.empty()) {
            components.push_back(ViewComponents::text(s));
        }
        this->str("");
        return 0;
    }

    ViewBuf() : std::stringbuf(std::ios_base::out) {}
    ViewBuf(const ViewBuf &rhs) : ViewBuf() { *this = rhs; }
    ViewBuf &operator=(const ViewBuf &rhs) {
        this->components = rhs.components;
        return *this;
    }
};

//! Viewの送受信データを表すクラス
/*! コンストラクタではなく Member::view() を使って取得してください
 */
class View : protected Field, public EventTarget<View>, public std::ostream {
    ViewBuf sb;
    void onAppend() const override { tryGet(); }

    //! 値をセットし、EventTargetを発動する
    WEBCFACE_DLL View &set(std::vector<ViewComponent> &v);
    WEBCFACE_DLL std::optional<std::shared_ptr<std::vector<ViewComponentBase>>>
    getRaw() const;

  public:
    View() : Field(), EventTarget<View>(), sb(), std::ostream(&sb) {}
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
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするValue
     */
    View child(const std::string &field) {
        return View{*this, this->field_ + "." + field};
    }
    //! Viewを取得する
    std::optional<std::vector<ViewComponent>> tryGet() const {
        auto vb = getRaw();
        if (vb) {
            std::vector<ViewComponent> v((*vb)->size());
            for (std::size_t i = 0; i < (*vb)->size(); i++) {
                v[i] = ViewComponent{(**vb)[i], this->data_w};
            }
            return v;
        } else {
            return std::nullopt;
        }
    }
    //! Viewを取得する
    std::vector<ViewComponent> get() const {
        return tryGet().value_or(std::vector<ViewComponent>{});
    }
    //! syncの時刻を返す
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL View &free();

    //! このViewのViewBufの内容を初期化する (コンストラクタ内でも自動で呼ばれる)
    View &init() {
        sb.components.clear();
        return *this;
    }
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
    View &operator<<(const Common::ViewComponentBase &vc) {
        setCheck();
        std::flush(*this);
        sb.components.push_back(ViewComponent{vc, this->data_w});
        return *this;
    }
    //! コンポーネントを追加
    View &operator<<(const ViewComponent &vc) {
        setCheck();
        std::flush(*this);
        sb.components.push_back(vc);
        return *this;
    }
    //! コンポーネントなどを追加 (operator<< と同じ)
    template <typename T>
    View &add(const T &rhs) {
        *this << rhs;
        return *this;
    }

    //! Viewの内容をclientに反映し送信可能にする (デストラクタで自動で呼ばれる)
    View &sync() {
        std::flush(*this);
        return set(sb.components);
    }
};
} // namespace webcface
