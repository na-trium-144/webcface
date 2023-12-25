#include <webcface/text.h>
#include "client_internal.h"
#include "../message/message.h"

namespace WEBCFACE_NS {
Text::Text(const Field &base)
    : Field(base), EventTarget<Text>(&this->dataLock()->text_change_event,
                                     *this) {}

inline void addTextReq(const std::shared_ptr<Internal::ClientData> &data,
                       const std::string &member_, const std::string &field_) {
    auto req = data->text_store.addReq(member_, field_);
    if (req) {
        data->message_queue->push(Message::packSingle(
            Message::Req<Message::Text>{{}, member_, field_, req}));
    }
}

Text &Text::set(const Text::Dict &v) {
    if (v.hasValue()) {
        setCheck()->text_store.setSend(*this, v.getRaw());
        this->triggerEvent(*this);
    } else {
        for (const auto &it : v.getChildren()) {
            child(it.first).set(it.second);
        }
    }
    return *this;
}
Text &Text::set(const std::string &v) {
    setCheck()->text_store.setSend(*this, std::make_shared<std::string>(v));
    this->triggerEvent(*this);
    return *this;
}

void Text::onAppend() const { addTextReq(dataLock(), member_, field_); }

std::optional<std::string> Text::tryGet() const {
    auto v = dataLock()->text_store.getRecv(*this);
    addTextReq(dataLock(), member_, field_);
    if (v) {
        return **v;
    } else {
        return std::nullopt;
    }
}
std::optional<Text::Dict> Text::tryGetRecurse() const {
    addTextReq(dataLock(), member_, field_);
    return dataLock()->text_store.getRecvRecurse(
        *this, [this](const std::string &subfield) {
            addTextReq(dataLock(), member_, subfield);
        });
}
std::chrono::system_clock::time_point Text::time() const {
    return dataLock()
        ->sync_time_store.getRecv(this->member_)
        .value_or(std::chrono::system_clock::time_point());
}
Text &Text::free() {
    auto req = dataLock()->text_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

} // namespace WEBCFACE_NS
