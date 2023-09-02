#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <cassert>
#include <memory>
#include "common/view.h"
#include "data.h"
#include "client_data.h"

namespace WebCFace {

class ViewComponent : public ViewComponentBase {
    std::weak_ptr<ClientData> data_w;

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

  public:
    ViewComponent() = default;
    ViewComponent(const ViewComponentBase &vc,
                  const std::weak_ptr<ClientData> &data_w)
        : ViewComponentBase(vc), data_w(data_w) {}
    explicit ViewComponent(ViewComponentType type) { type_ = type; }

    ViewComponent &lockTmp(const std::weak_ptr<ClientData> &data_w,
                           const std::string &field_id) {
        if (on_click_func_tmp != nullptr) {
            auto data = data_w.lock();
            Func on_click{Field{data_w, data->self_member_name}, field_id};
            on_click_func_tmp->lockTo(on_click);
            onClick(on_click);
        }
        return *this;
    }

    ViewComponentType type() const { return type_; }
    std::string text() const { return text_; }
    ViewComponent &text(const std::string &text) {
        text_ = text;
        return *this;
    }
    std::optional<Func> onClick() const {
        if (on_click_func_ != std::nullopt) {
            assert(data_w != std::nullptr && "ClientData not set");
            return Field{data_w, on_click_func_->member_,
                         on_click_func_->field_};
        } else {
            return std::nullopt;
        }
    }
    ViewComponent &onClick(const Func &func) {
        // data_w = static_cast<Field>(func).data_w;
        // on_click_func_ = static_cast<FieldBase>(func);
        on_click_func_ = FieldBase{func.member().name(), func.name()};
        return *this;
    }
    template <typename T>
    ViewComponent &onClick(const T &func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
    ViewColor textColor() const { return text_color_; }
    ViewComponent &textColor(ViewColor c) {
        text_color_ = c;
        return *this;
    }
    ViewColor bgColor() const { return bg_color_; }
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

class ViewBuf : public std::stringbuf {
  public:
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
    ViewBuf &operator=(const ViewBuf &rhs) {
        this->components = rhs.components;
        return *this;
    }
};

class View : protected Field, public EventTarget<View>, public std::ostream {
    ViewBuf sb;

  public:
    View() : Field(), EventTarget<View>(), sb(), std::ostream(&sb) {}
    View(const Field &base)
        : Field(base), EventTarget<View>(EventType::view_change, base,
                                         [this] { this->tryGet(); }),
          sb(), std::ostream(&sb) {
        init();
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

    ~View() { sync(); }

    using Field::member;
    using Field::name;

  private:
    //! 値をセットし、EventTargetを発動する
    auto &set(std::vector<ViewComponent> &v) {
        std::vector<ViewComponentBase> vb(v.size());
        for (std::size_t i = 0; i < v.size(); i++) {
            vb[i] = v[i].lockTmp(this->data_w,
                                 this->name() + "_" + std::to_string(i));
        }
        setCheck();
        dataLock()->view_store.setSend(*this, vb);
        triggerEvent();
        return *this;
    }

  public:
    //! 値を取得する
    std::optional<std::vector<ViewComponent>> tryGet() const {
        auto vb = dataLock()->view_store.getRecv(*this);
        if (vb) {
            std::optional<std::vector<ViewComponent>> v(vb->size());
            for (std::size_t i = 0; i < vb->size(); i++) {
                v->at(i) = ViewComponent{vb->at(i), this->data_w};
            }
            return v;
        } else {
            return std::nullopt;
        }
    }
    std::vector<ViewComponent> get() const {
        return tryGet().value_or(std::vector<ViewComponent>{});
    }

    //! このviewを非表示にする
    //! (他clientのentryに表示されなくする)
    auto &hidden(bool hidden) {
        setCheck();
        dataLock()->view_store.setHidden(*this, hidden);
        return *this;
    }
    //! 関数の設定を解除
    auto &free() {
        dataLock()->view_store.unsetRecv(*this);
        return *this;
    }

    View &init() {
        setCheck();
        sb.components.clear();
        return *this;
    }
    template <typename T>
    View &operator<<(const T &rhs) {
        static_cast<std::ostream &>(*this) << rhs << std::flush;
        return *this;
    }
    View &operator<<(std::ostream &(*os_manip)(std::ostream &)) {
        os_manip(*this);
        return *this;
    }

    View &operator<<(const ViewComponentBase &vc) {
        setCheck();
        sb.components.push_back(ViewComponent{vc, this->data_w});
        return *this;
    }
    View &operator<<(const ViewComponent &vc) {
        setCheck();
        sb.components.push_back(vc);
        return *this;
    }
    template <typename T>
    View &add(const T &rhs) {
        *this << rhs;
        return *this;
    }
    View &sync() { return set(sb.components); }
};
} // namespace WebCFace