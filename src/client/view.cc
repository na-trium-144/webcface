#include <webcface/view.h>
#include "client_internal.h"
#include <webcface/member.h>
#include "../message/message.h"

namespace webcface {
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
        // Fieldの中でnullptrは処理してくれるからいいかな
        // assert(data_w.lock() != nullptr && "ClientData not set");
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
ViewComponent &ViewComponent::onClick(const Func &func) {
    on_click_func_ = FieldBase{func.member().name(), func.name()};
    return *this;
}


View::View()
    : Field(), EventTarget<View>(),
      sb(std::make_shared<ViewBuf>()), std::ostream(nullptr) {
    this->std::ostream::init(sb.get());
}
View::View(const Field &base)
    : Field(base), EventTarget<View>(&this->dataLock()->view_change_event,
                                     *this),
      sb(std::make_shared<ViewBuf>()), std::ostream(nullptr) {
    this->std::ostream::init(sb.get());
}
View &View::init() {
    sb->components.clear();
    sb->modified = true;
    return *this;
}
View &View::sync() {
    std::flush(*this);
    if (sb->modified) {
        set(sb->components);
        sb->modified = false;
    }
    return *this;
}
void View::onDestroy() {
    if (sb.use_count() == 1 && data_w.lock() != nullptr &&
        dataLock()->isSelf(member_)) {
        sync();
    }
    this->rdbuf(nullptr);
}
View &View::operator<<(const ViewComponent &vc) {
    std::flush(*this);
    sb->components.push_back(vc);
    sb->modified = true;
    return *this;
}
int ViewBuf::sync() {
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
        modified = true;
    }
    if (!s.empty()) {
        components.push_back(ViewComponents::text(s));
        modified = true;
    }
    this->str("");
    return 0;
}
View &View::operator<<(const View &vg) {
    std::flush(*this);
    if (vg.sb->modified || !vg.sb->components.empty()) {
        for (const auto &vc : vg.sb->components) {
            this->sb->components.push_back(vc);
        }
    } else if (vg.data_w.lock() != nullptr) {
        for (const auto &vc : vg.get()) {
            this->sb->components.push_back(vc);
        }
    }
    return *this;
}

View &View::operator=(const View &rhs) {
    onDestroy();
    this->Field::operator=(rhs);
    this->EventTarget<View>::operator=(rhs);
    this->sb = rhs.sb;
    this->rdbuf(sb.get());
    return *this;
}
View &View::operator=(View &&rhs) {
    onDestroy();
    this->Field::operator=(std::move(rhs));
    this->EventTarget<View>::operator=(std::move(rhs));
    this->sb = std::move(rhs.sb);
    this->rdbuf(sb.get());
    return *this;
}
View &View::set(std::vector<ViewComponent> &v) {
    setCheck();
    std::vector<ViewComponentBase> vb(v.size());
    for (std::size_t i = 0; i < v.size(); i++) {
        vb[i] =
            v[i].lockTmp(this->data_w, this->name() + "_" + std::to_string(i));
    }
    setCheck()->view_store.setSend(
        *this, std::make_shared<std::vector<ViewComponentBase>>(vb));
    triggerEvent(*this);
    return *this;
}

inline void addViewReq(const std::shared_ptr<Internal::ClientData> &data,
                       const std::string &member_, const std::string &field_) {
    auto req = data->view_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::View>{{}, member_, field_, req}));
    }
}
void View::onAppend() const { addViewReq(dataLock(), member_, field_); }
std::optional<std::vector<ViewComponent>> View::tryGet() const {
    auto vb = dataLock()->view_store.getRecv(*this);
    addViewReq(dataLock(), member_, field_);
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
std::chrono::system_clock::time_point View::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
View &View::free() {
    auto req = dataLock()->view_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace webcface
