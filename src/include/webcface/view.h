#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <cassert>
#include <memory>
#include "common/view.h"
#include "data.h"
#include "client_data.h"
#include "func.h"

namespace WebCFace {

//! Viewに表示する要素です
class ViewComponent : public ViewComponentBase {
    std::weak_ptr<ClientData> data_w;

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

  public:
    ViewComponent() = default;
    ViewComponent(const ViewComponentBase &vc,
                  const std::weak_ptr<ClientData> &data_w)
        : ViewComponentBase(vc), data_w(data_w) {}
    explicit ViewComponent(ViewComponentType type) { type_ = type; }

    //! AnonymousFuncをFuncオブジェクトにlockします
    ViewComponent &lockTmp(const std::weak_ptr<ClientData> &data_w,
                           const std::string &field_id) {
        if (on_click_func_tmp != nullptr) {
            auto data = data_w.lock();
            Func on_click{Field{data_w, data->self_member_name}, field_id};
            on_click_func_tmp->lockTo(on_click);
            on_click.hidden(true);
            onClick(on_click);
        }
        return *this;
    }

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
    std::optional<Func> onClick() const {
        if (on_click_func_ != std::nullopt) {
            assert(data_w.lock() != nullptr && "ClientData not set");
            return Field{data_w, on_click_func_->member_,
                         on_click_func_->field_};
        } else {
            return std::nullopt;
        }
    }
    //! クリック時に実行される関数を設定
    ViewComponent &onClick(const Func &func) {
        // data_w = static_cast<Field>(func).data_w;
        // on_click_func_ = static_cast<FieldBase>(func);
        on_click_func_ = FieldBase{func.member().name(), func.name()};
        return *this;
    }
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
inline ViewComponent text(const std::string &text) {
    return ViewComponent(ViewComponentType::text).text(text);
}
inline ViewComponent newLine() {
    return ViewComponent(ViewComponentType::new_line);
}
template <typename T>
ViewComponent button(const std::string &text, const T &func) {
    return ViewComponent(ViewComponentType::button).text(text).onClick(func);
}
} // namespace ViewComponents

//! Viewのstd::ostreamにデータが追加された際にそれをViewComponentに変換するためのstreambuf
class ViewBuf : public std::stringbuf {
  public:
    //! 送信用のデータ
    std::vector<ViewComponent> components;
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
 * 
 * 送信用viewデータは ViewBuf 内で保持するが、受信データはtryGet()時に取得するので別
 */
class View : protected Field, public EventTarget<View>, public std::ostream {
    ViewBuf sb;
    void onAppend() const override { tryGet(); }

    //! 値をセットし、EventTargetを発動する
    auto &set(std::vector<ViewComponent> &v) {
        std::vector<ViewComponentBase> vb(v.size());
        for (std::size_t i = 0; i < v.size(); i++) {
            vb[i] = v[i].lockTmp(this->data_w,
                                 this->name() + "_" + std::to_string(i));
        }
        setCheck();
        dataLock()->view_store.setSend(
            *this, std::make_shared<std::vector<ViewComponentBase>>(vb));
        triggerEvent(*this);
        return *this;
    }

  public:
    View() : Field(), EventTarget<View>(), sb(), std::ostream(&sb) {}
    View(const Field &base)
        : Field(base), EventTarget<View>(&this->dataLock()->view_change_event,
                                         *this),
          sb(), std::ostream(&sb) {

        if (dataLock()->isSelf(member_)) {
            init();
        }
    }
    View(const Field &base, const std::string &field)
        : View(Field{base, field}) {}
    View(const View &rhs) : View() { *this = rhs; }
    View &operator=(const View &rhs) {
        this->Field::operator=(rhs);
        this->EventTarget<View>::operator=(rhs);
        this->sb = rhs.sb;
        return *this;
    }

    ~View() {
        if (dataLock()->isSelf(member_)) {
            sync();
        }
    }

    using Field::member;
    using Field::name;

    //! 子要素を返す
    View child(const std::string &field) {
        return View{*this, this->field_ + "." + field};
    }
    //! 値を取得する
    std::optional<std::vector<ViewComponent>> tryGet() const {
        auto vb = dataLock()->view_store.getRecv(*this);
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
    std::vector<ViewComponent> get() const {
        return tryGet().value_or(std::vector<ViewComponent>{});
    }
    std::chrono::system_clock::time_point time() const {
        return dataLock()
            ->sync_time_store.getRecv(this->member_)
            .value_or(std::chrono::system_clock::time_point());
    }

    // //! このviewを非表示にする
    // //! (他clientのentryに表示されなくする)
    // auto &hidden(bool hidden) {
    //     setCheck();
    //     dataLock()->view_store.setHidden(*this, hidden);
    //     return *this;
    // }

    //! 値やリクエスト状態をクリア
    View &free() {
        dataLock()->view_store.unsetRecv(*this);
        return *this;
    }

    //! このViewのViewBufの内容を初期化する (コンストラクタ内で自動で呼ばれる)
    View &init() {
        sb.components.clear();
        return *this;
    }
    //! 文字列にフォーマット & flushし、textコンポーネントとして追加
    template <typename T>
    View &operator<<(const T &rhs) {
        static_cast<std::ostream &>(*this) << rhs << std::flush;
        return *this;
    }
    View &operator<<(std::ostream &(*os_manip)(std::ostream &)) {
        os_manip(*this);
        return *this;
    }
    //! コンポーネントを追加
    View &operator<<(const ViewComponentBase &vc) {
        setCheck();
        sb.components.push_back(ViewComponent{vc, this->data_w});
        return *this;
    }
    //! コンポーネントを追加
    View &operator<<(const ViewComponent &vc) {
        setCheck();
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
    View &sync() { return set(sb.components); }
};
} // namespace WebCFace