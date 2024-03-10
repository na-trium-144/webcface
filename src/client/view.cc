#include <webcface/view.h>
#include "client_internal.h"
#include <webcface/member.h>
#include "../message/message.h"
#include "data_buffer.h"

namespace WEBCFACE_NS {

template class WEBCFACE_DLL EventTarget<View>;

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
wcfViewComponent ViewComponent::cData() const {
    wcfViewComponent vcc;
    vcc.type = static_cast<int>(this->type());
    vcc.text = this->text_.empty() ? nullptr : this->text_.c_str();
    if (this->on_click_func_) {
        vcc.on_click_member = this->on_click_func_->member_.c_str();
        vcc.on_click_field = this->on_click_func_->field_.c_str();
    } else {
        vcc.on_click_member = nullptr;
        vcc.on_click_field = nullptr;
    }
    vcc.text_color = static_cast<int>(this->text_color_);
    vcc.bg_color = static_cast<int>(this->bg_color_);
    return vcc;
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
    : Field(), EventTarget<View>(), std::ostream(nullptr),
      sb(std::make_shared<Internal::ViewBuf>()) {
    this->std::ostream::init(sb.get());
}
View::View(const Field &base)
    : Field(base), EventTarget<View>(&this->dataLock()->view_change_event,
                                     *this),
      std::ostream(nullptr), sb(std::make_shared<Internal::ViewBuf>(base)) {
    this->std::ostream::init(sb.get());
}
View::~View() { this->rdbuf(nullptr); }

Internal::ViewBuf::ViewBuf()
    : std::stringbuf(std::ios_base::out), DataSetBuffer<ViewComponent>() {}
Internal::ViewBuf::ViewBuf(const Field &base)
    : std::stringbuf(std::ios_base::out), DataSetBuffer<ViewComponent>(base) {}
Internal::ViewBuf::~ViewBuf() { sync(); }

View &View::init() {
    sb->init();
    return *this;
}
View &View::sync() {
    std::flush(*this);
    sb->syncSetBuf();
    return *this;
}
template <>
void Internal::DataSetBuffer<ViewComponent>::sync() {
    if (modified_) {
        modified_ = false;
        auto vb = std::make_shared<std::vector<ViewComponentBase>>();
        vb->reserve(components_.size());
        for (std::size_t i = 0; i < components_.size(); i++) {
            vb->push_back(std::move(components_[i].lockTmp(
                target_.data_w, target_.name() + "_" + std::to_string(i))));
        }
        target_.setCheck()->view_store.setSend(target_, vb);
        static_cast<View>(target_).triggerEvent(target_);
    }
}

View &View::operator<<(ViewComponent &vc) {
    std::flush(*this);
    sb->addVC(vc);
    return *this;
}
View &View::operator<<(ViewComponent &&vc) {
    std::flush(*this);
    sb->addVC(std::move(vc));
    return *this;
}
void Internal::ViewBuf::addText(const ViewComponent &vc) {
    std::string s = vc.text();
    while (true) {
        auto p = s.find('\n');
        if (p == std::string::npos) {
            break;
        }
        std::string c1 = s.substr(0, p);
        if (!c1.empty()) {
            ViewComponent vc_new = vc;
            vc_new.text(c1);
            add(std::move(vc_new));
        }
        add(ViewComponents::newLine());
        s = s.substr(p + 1);
    }
    if (!s.empty()) {
        ViewComponent vc_new = vc;
        vc_new.text(s);
        add(std::move(vc_new));
    }
}
void Internal::ViewBuf::addVC(const ViewComponent &vc) {
    if (vc.type() == ViewComponentType::text) {
        addText(vc);
    } else {
        add(vc);
    }
}
void Internal::ViewBuf::addVC(ViewComponent &&vc) {
    if (vc.type() == ViewComponentType::text) {
        addText(vc);
    } else {
        add(std::move(vc));
    }
}
int Internal::ViewBuf::sync() {
    if (!this->str().empty()) {
        this->addText(ViewComponents::text(this->str()));
        this->str("");
    }
    return 0;
}

View &View::operator=(const View &rhs) {
    this->Field::operator=(rhs);
    this->EventTarget<View>::operator=(rhs);
    this->sb = rhs.sb;
    this->rdbuf(sb.get());
    return *this;
}
View &View::operator=(View &&rhs) {
    this->Field::operator=(std::move(rhs));
    this->EventTarget<View>::operator=(std::move(rhs));
    this->sb = std::move(rhs.sb);
    this->rdbuf(sb.get());
    return *this;
}

void View::request() const {
    auto data = dataLock();
    auto req = data->view_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::View>{{}, member_, field_, req}));
    }
}
void View::onAppend() const { request(); }
std::optional<std::vector<ViewComponent>> View::tryGet() const {
    request();
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
std::chrono::system_clock::time_point View::time() const {
    return member().syncTime();
}
View &View::free() {
    auto req = dataLock()->view_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace WEBCFACE_NS
