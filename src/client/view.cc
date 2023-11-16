#include <webcface/view.h>
#include "client_internal.h"
#include <webcface/member.h>

namespace WebCFace {
ViewComponentBase &
ViewComponent::lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
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
std::optional<Func> ViewComponent::onClick() const {
    if (on_click_func_ != std::nullopt) {
        assert(data_w.lock() != nullptr && "ClientData not set");
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
ViewComponent &ViewComponent::onClick(const Func &func) {
    on_click_func_ = FieldBase{func.member().name(), func.name()};
    return *this;
}


View::View(const Field &base)
    : Field(base), EventTarget<View>(&this->dataLock()->view_change_event,
                                     *this),
      sb(), std::ostream(&sb) {

    if (dataLock()->isSelf(member_)) {
        init();
    }
}
View::~View() {
    if (dataLock()->isSelf(member_)) {
        sync();
    }
}

View &View::operator=(const View &rhs) {
    this->Field::operator=(rhs);
    this->EventTarget<View>::operator=(rhs);
    this->sb = rhs.sb;
    return *this;
}
View &View::set(std::vector<ViewComponent> &v) {
    std::vector<ViewComponentBase> vb(v.size());
    for (std::size_t i = 0; i < v.size(); i++) {
        vb[i] =
            v[i].lockTmp(this->data_w, this->name() + "_" + std::to_string(i));
    }
    setCheck();
    dataLock()->view_store.setSend(
        *this, std::make_shared<std::vector<ViewComponentBase>>(vb));
    triggerEvent(*this);
    return *this;
}
std::optional<std::shared_ptr<std::vector<ViewComponentBase>>>
View::getRaw() const {
    return dataLock()->view_store.getRecv(*this);
}
std::chrono::system_clock::time_point View::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
View &View::free() {
    dataLock()->view_store.unsetRecv(*this);
    return *this;
}

} // namespace WebCFace