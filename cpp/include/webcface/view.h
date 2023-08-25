#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include "common/view.h"
#include "data.h"

namespace WebCFace {

class ViewBuf : public FieldBase, public std::stringbuf {
  public:
    std::vector<ViewComponent> components;
    int sync() override {
        setCheck();
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

    ViewBuf() : FieldBase(), std::stringbuf(std::ios_base::out) {}
    explicit ViewBuf(const FieldBase &base)
        : FieldBase(base), std::stringbuf(std::ios_base::out) {}
    ViewBuf &operator=(const ViewBuf &rhs) {
        this->FieldBase::operator=(rhs);
        this->components = rhs.components;
        return *this;
    }
};

class View : public SyncFieldBase<std::vector<ViewComponent>>,
             public EventTarget<View>,
             public std::ostream {
    using SyncFieldBase<std::vector<ViewComponent>>::FieldBase::dataLock;
    using SyncFieldBase<std::vector<ViewComponent>>::FieldBase::setCheck;
    ViewBuf sb;

  public:
    View()
        : SyncFieldBase<std::vector<ViewComponent>>(),
          EventTarget<View>(), std::ostream() {}
    View(const FieldBase &base)
        : SyncFieldBase<std::vector<ViewComponent>>(base),
          EventTarget<View>(EventType::view_change, base,
                            [this] { this->tryGet(); }),
          sb(base), std::ostream(&sb) {
        init();
    }
    View(const FieldBase &base, const std::string &field)
        : View(FieldBase{base, field}) {}
    View(const View &rhs)
        : View(
              static_cast<SyncFieldBase<std::vector<ViewComponent>>::FieldBase>(
                  rhs)) {}
    View &operator=(const View &rhs) {
        this->SyncFieldBase<std::vector<ViewComponent>>::operator=(rhs);
        this->EventTarget<View>::operator=(rhs);
        this->sb = rhs.sb;
        return *this;
    }

    ~View() { end(); }

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
    std::optional<std::vector<ViewComponent>> tryGet() const override {
        return dataLock()->view_store.getRecv(*this);
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