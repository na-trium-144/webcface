#include "webcface/view.h"
#include "webcface/internal/client_internal.h"
#include "webcface/member.h"
#include "webcface/message/message.h"
#include "webcface/internal/data_buffer.h"

WEBCFACE_NS_BEGIN

View::View()
    : Field(), std::ostream(nullptr),
      sb(std::make_shared<internal::ViewBuf>()) {
    this->std::ostream::init(sb.get());
}
View::View(const Field &base)
    : Field(base), std::ostream(nullptr),
      sb(std::make_shared<internal::ViewBuf>(base)) {
    this->std::ostream::init(sb.get());
}
View::~View() { this->rdbuf(nullptr); }

internal::ViewBuf::ViewBuf()
    : std::stringbuf(std::ios_base::out), DataSetBuffer<ViewComponent>() {}
internal::ViewBuf::ViewBuf(const Field &base)
    : std::stringbuf(std::ios_base::out), DataSetBuffer<ViewComponent>(base) {}
internal::ViewBuf::~ViewBuf() { sync(); }

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
void internal::DataSetBuffer<ViewComponent>::onSync() {
    std::unordered_map<int, int> idx_next;
    auto data = target_.setCheck();
    for (std::size_t i = 0; i < components_.size(); i++) {
        components_[i].lockTmp(data, target_.field_, &idx_next);
    }
    data->view_store.setSend(
        target_,
        std::make_shared<std::vector<ViewComponent>>(std::move(components_)));
    std::shared_ptr<std::function<void(View)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event = data->view_change_event[target_.member_][target_.field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
View &View::onChange(std::function<void(View)> callback) {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->view_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(View)>>(std::move(callback));
    return *this;
}

View &View::operator<<(const ViewComponent &vc) {
    std::flush(*this);
    sb->addVC(vc);
    return *this;
}
View &View::operator<<(ViewComponent &&vc) {
    std::flush(*this);
    sb->addVC(std::move(vc));
    return *this;
}
void internal::ViewBuf::addText(const ViewComponent &vc) {
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
void internal::ViewBuf::addVC(const ViewComponent &vc) {
    if (vc.type() == ViewComponentType::text) {
        addText(vc);
    } else {
        add(vc);
    }
}
void internal::ViewBuf::addVC(ViewComponent &&vc) {
    if (vc.type() == ViewComponentType::text) {
        addText(vc);
    } else {
        add(std::move(vc));
    }
}
int internal::ViewBuf::sync() {
    if (!this->str().empty()) {
        this->addText(ViewComponents::text(this->str()).toV());
        this->str("");
    }
    return 0;
}

View &View::operator=(const View &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->Field::operator=(rhs);
    this->sb = rhs.sb;
    this->rdbuf(sb.get());
    return *this;
}
View &View::operator=(View &&rhs) noexcept {
    if (this == &rhs) {
        return *this;
    }
    this->Field::operator=(std::move(static_cast<Field &>(rhs)));
    this->sb = std::move(rhs.sb);
    this->rdbuf(sb.get());
    return *this;
}

void View::request() const {
    auto data = dataLock();
    auto req = data->view_store.addReq(member_, field_);
    if (req) {
        data->message_push(message::packSingle(
            message::Req<message::View>{{}, member_, field_, req}));
    }
}
std::optional<std::vector<ViewComponent>> View::tryGet() const {
    request();
    auto vb = dataLock()->view_store.getRecv(*this);
    if (vb) {
        std::vector<ViewComponent> v((*vb)->size());
        std::unordered_map<int, int> idx_next;
        for (std::size_t i = 0; i < (*vb)->size(); i++) {
            v[i] = ViewComponent{(**vb)[i], this->data_w, &idx_next};
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

WEBCFACE_NS_END
