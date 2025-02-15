#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/view.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/data_store2.h"
#include "webcface/member.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/component_internal.h"

WEBCFACE_NS_BEGIN

View::View()
    : Field(), sb(std::make_shared<internal::ViewBuf>()), os(sb.get()) {}
View::View(const Field &base)
    : Field(base), sb(std::make_shared<internal::ViewBuf>(base)), os(sb.get()) {
}
View::~View() { os.rdbuf(nullptr); }

internal::ViewBuf::ViewBuf()
    : std::stringbuf(std::ios_base::out),
      DataSetBuffer<TemporalViewComponent>() {}
internal::ViewBuf::ViewBuf(const Field &base)
    : std::stringbuf(std::ios_base::out),
      DataSetBuffer<TemporalViewComponent>(base) {}
internal::ViewBuf::~ViewBuf() { sync(); }

const View &View::init() const {
    sb->init();
    return *this;
}
const View &View::sync() const {
    std::flush(os);
    sb->syncSetBuf();
    return *this;
}
template <>
void internal::DataSetBuffer<TemporalViewComponent>::onSync() {
    std::unordered_map<ViewComponentType, int> idx_next;
    auto data = target_.setCheck();
    auto components_p = std::make_shared<message::ViewData>();
    components_p->data_ids.reserve(components_.size());
    for (std::size_t i = 0; i < components_.size(); i++) {
        std::shared_ptr<internal::TemporalViewComponentData> msg_data =
            components_[i].lockTmp(data, target_.field_, &idx_next);
        components_p->components.emplace(
            msg_data->id.u8String(),
            std::static_pointer_cast<message::ViewComponentData>(msg_data));
        components_p->data_ids.push_back(msg_data->id);
    }
    data->view_store.setSend(target_, components_p);

    auto change_event =
        internal::findFromMap2(data->view_change_event.shared_lock().get(),
                               target_.member_, target_.field_);
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
const View &View::onChange(std::function<void(View)> callback) const {
    this->request();
    auto data = dataLock();
    data->view_change_event.lock().get()[this->member_][this->field_] =
        std::make_shared<std::function<void(View)>>(std::move(callback));
    return *this;
}

const View &View::operator<<(TemporalViewComponent vc) const {
    std::flush(os);
    sb->addVC(std::move(vc));
    return *this;
}
void internal::ViewBuf::addText(std::string_view text,
                                const TemporalViewComponent *vc) {
    std::string_view sv = text;
    while (true) {
        auto p = sv.find('\n');
        if (p == std::string::npos) {
            break;
        }
        std::string_view c1 = sv.substr(0, p);
        if (!c1.empty()) {
            if (vc) {
                TemporalViewComponent tx = *vc;
                tx.text(c1);
                add(std::move(tx));
            } else {
                add(std::move(components::text(c1).component_v));
            }
        }
        add(components::newLine());
        sv = sv.substr(p + 1);
    }
    if (!sv.empty()) {
        if (vc) {
            TemporalViewComponent tx = *vc;
            tx.text(sv);
            add(std::move(tx));
        } else {
            add(std::move(components::text(sv).component_v));
        }
    }
}
void internal::ViewBuf::addVC(TemporalViewComponent &&vc) {
    if (vc.msg_data->type == static_cast<int>(ViewComponentType::text)) {
        addText(vc.msg_data->text.u8StringView(), &vc);
    } else {
        add(std::move(vc));
    }
}
int internal::ViewBuf::sync() {
    if (!this->str().empty()) {
        this->addText(this->str(), nullptr);
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
    os.rdbuf(sb.get());
    return *this;
}
View &View::operator=(View &&rhs) noexcept {
    if (this == &rhs) {
        return *this;
    }
    this->Field::operator=(std::move(static_cast<Field &>(rhs)));
    this->sb = std::move(rhs.sb);
    os.rdbuf(sb.get());
    return *this;
}

const View &View::request() const {
    auto data = dataLock();
    auto req = data->view_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(
            message::Req<message::View>{{}, member_, field_, req});
    }
    return *this;
}
std::optional<std::vector<ViewComponent>> View::tryGet() const {
    request();
    auto vb = dataLock()->view_store.getRecv(*this);
    if (vb) {
        std::vector<ViewComponent> v;
        v.reserve((*vb)->data_ids.size());
        for (const auto &id : (*vb)->data_ids) {
            v.emplace_back((*vb)->components.at(id.u8String()), this->data_w,
                           id);
        }
        return v;
    } else {
        return std::nullopt;
    }
}
std::chrono::system_clock::time_point View::time() const {
    return member().syncTime();
}
const View &View::free() const {
    auto req = dataLock()->view_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}
bool View::exists() const {
    return dataLock()->view_store.getEntry(member_).count(field_);
}

WEBCFACE_NS_END
