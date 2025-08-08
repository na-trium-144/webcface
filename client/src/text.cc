#include "webcface/common/internal/message/pack.h"
#include "webcface/text.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/member.h"
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN

namespace internal {
struct InputRefState {
    Variant field;
    InputRefState() = default;
};
} // namespace internal

Variant::Variant(const Field &base) : Field(base) {}

InputRef::InputRef() : state(std::make_shared<internal::InputRefState>()) {}
void InputRef::lockTo(const Variant &target) { state->field = target; }
Variant &InputRef::lockedField() const { return state->field; }
ValAdaptor InputRef::get() const {
    if (lockedField().expired()) {
        return ValAdaptor();
    } else {
        return lockedField().get();
    }
    /*if (expired()) {
        if (!state->val.empty()) {
            state->val = ValAdaptor();
        }
    } else {
        auto new_val = state->field.get();
        if (state->val != new_val) {
            state->val = new_val;
        }
    }
    return state->val;*/
}

const Variant &Variant::request() const {
    auto data = dataLock();
    auto req = data->text_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(
            message::Req<message::Text>{{}, member_, field_, req});
    }
    return *this;
}

const Variant &Variant::set(const ValAdaptor &v) const {
    auto data = setCheck();
    data->text_store.setSend(*this, v);
    auto change_event =
        internal::findFromMap2(data->text_change_event.shared_lock().get(),
                               this->member_, this->field_);
    if (change_event && *change_event) {
        change_event->operator()(*this);
    }
    return *this;
}
const Variant &Variant::onChange(std::function<void(Variant)> callback) const {
    this->request();
    auto data = dataLock();
    data->text_change_event.lock().get()[this->member_][this->field_] =
        std::make_shared<std::function<void(Variant)>>(std::move(callback));
    return *this;
}

std::optional<ValAdaptor> Variant::tryGet() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return *v;
    } else {
        return std::nullopt;
    }
}
ValAdaptor Variant::get() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return *v;
    } else {
        return ValAdaptor();
    }
}
std::optional<StringView> Text::tryGet() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return v->asStringView();
    } else {
        return std::nullopt;
    }
}
StringView Text::get() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return v->asStringView();
    } else {
        return StringView{};
    }
}
std::optional<WStringView> Text::tryGetW() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return v->asWStringView();
    } else {
        return std::nullopt;
    }
}
WStringView Text::getW() const {
    auto v = dataLock()->text_store.getRecv(*this);
    request();
    if (v) {
        return v->asWStringView();
    } else {
        return WStringView{};
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
const Text &Text::free() const {
    auto req = dataLock()->text_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}

bool Text::exists() const {
    return dataLock()->text_store.getEntry(member_).count(field_);
}

std::ostream &operator<<(std::ostream &os, const Text &data) {
    return os << data.get();
}
WEBCFACE_NS_END
