#include <webcface/text.h>
#include <webcface/member.h>
#include "webcface/internal/client_internal.h"
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN

Text::Text(const Field &base) : Field(base) {}

void Text::request() const {
    auto data = dataLock();
    auto req = data->text_store.addReq(member_, field_);
    if (req) {
        data->message_push(message::packSingle(
            message::Req<message::Text>{{}, member_, field_, req}));
    }
}

Text &Text::set(const ValAdaptor &v) {
    auto data = setCheck();
    data->text_store.setSend(*this, std::make_shared<ValAdaptor>(v));
    std::shared_ptr<std::function<void(Text)>> change_event;
    {
        std::lock_guard lock(data->event_m);
        change_event = data->text_change_event[this->member_][this->field_];
    }
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}
Text &Text::onChange(std::function<void(Text)> callback) {
    this->request();
    auto data = dataLock();
    std::lock_guard lock(data->event_m);
    data->text_change_event[this->member_][this->field_] =
        std::make_shared<std::function<void(Text)>>(std::move(callback));
    return *this;
}

std::optional<ValAdaptor> Text::tryGetV() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return **v;
    } else {
        return std::nullopt;
    }
}
std::optional<std::string> Text::tryGet() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return (*v)->asString();
    } else {
        return std::nullopt;
    }
}
std::optional<std::wstring> Text::tryGetW() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return (*v)->asWString();
    } else {
        return std::nullopt;
    }
}

// std::optional<Text::Dict> Text::tryGetRecurse() const {
//     request();
//     return dataLock()->text_store.getRecvRecurse(
//         *this,
//         [this](const std::string &subfield) { child(subfield).request(); });
// }
std::chrono::system_clock::time_point Text::time() const {
    return member().syncTime();
}
Text &Text::free() {
    auto req = dataLock()->text_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

std::ostream &operator<<(std::ostream &os, const Text &data) {
    return os << data.get();
}
WEBCFACE_NS_END
