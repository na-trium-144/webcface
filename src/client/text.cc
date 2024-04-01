#include <webcface/text.h>
#include <webcface/member.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {

template class WEBCFACE_DLL EventTarget<Text>;

Text::Text(const Field &base)
    : Field(base), EventTarget<Text>(&this->dataLock()->text_change_event,
                                     *this) {}

void Text::request() const {
    auto data = dataLock();
    auto req = data->text_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Text>{{}, member_, field_, req}));
    }
}

// Text &Text::set(const Text::Dict &v) {
//     if (v.hasValue()) {
//         setCheck()->text_store.setSend(*this, v.getRaw());
//         this->triggerEvent(*this);
//     } else {
//         for (const auto &it : v.getChildren()) {
//             child(it.first).set(it.second);
//         }
//     }
//     return *this;
// }
Text &Text::set(const ValAdaptor &v) {
    setCheck()->text_store.setSend(*this, std::make_shared<ValAdaptor>(v));
    this->triggerEvent(*this);
    return *this;
}

void Text::onAppend() const { request(); }

std::optional<ValAdaptor> Text::tryGet() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return **v;
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
} // namespace WEBCFACE_NS
