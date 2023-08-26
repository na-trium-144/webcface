#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include "common/view.h"
#include "data.h"
#include "client_data.h";

namespace WebCFace {

class ViewComponent : protected ViewComponentBase {
    std::weak_ptr<ClientData> data_w;

  public:
    ViewComponent() = default;
    ViewComponent(const ViewComponentBase &vc,
                  const std::weak_ptr<ClientData> &data_w)
        : ViewComponentBase(vc), data_w(data_w) {}
    explicit ViewComponent(ViewComponentType type) { this->type_ = type; }
    ViewComponent(const std::string &text) {
        this->type_ = ViewComponentType::text;
        this->text_ = text;
    }

    ViewComponentType type() const { return type_; }
    std::string text() const { return text_; }
    void text(const std::string &text) { text_ = text; }
    Func onClick() const {
        return Func{data_w, on_click_func_.member_, on_click_func_.field_};
    }
    void onClick(const Func &func) { on_click_func_ = func; }
    ViewColor textColor() const { return view_color_; }
    // todo


    inline static ViewComponent newLine() {
        return ViewComponent(ViewComponentType::new_line);
    }
};

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
                components.push_back(c1);
            }
            components.push_back(ViewComponents::newLine());
            s = s.substr(p + 1);
        }
        if (!s.empty()) {
            components.push_back(s);
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

    ~View() { end(); }

    using Field::member;
    using Field::name;

  private:
    //! 値をセットし、EventTargetを発動する
    auto &set(const std::vector<ViewComponent> &v) {
        setCheck();
        dataLock()->view_store.setSend(*this, v);
        triggerEvent();
        return *this;
    }

  public:
    //! 値を取得する
    std::optional<std::vector<ViewComponent>> tryGet() const {
        return dataLock()->view_store.getRecv(*this);
    }
    std::vector<ViewComponent> get() const {
        return tryGet().value_or(std::vector<ViewComponent>{});
    }

    View &init() {
        setCheck();
        sb.components.clear();
        return *this;
    }
    template <typename T>
    View &operator<<(const T &rhs) {
        static_cast<std::ostream &>(*this) << rhs;
        return *this;
    }
    View &operator<<(std::ostream &(*os_manip)(std::ostream &)) {
        os_manip(*this);
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
    View &end() { return set(sb.components); }
};
} // namespace WebCFace