#include <webcface/field.h>
#include <webcface/member.h>
#include "client_internal.h"
#include <stdexcept>
#include <webcface/common/def.h>
#include <webcface/encoding.h>

WEBCFACE_NS_BEGIN

MemberNameRef Field::getMemberRef(std::weak_ptr<Internal::ClientData> data_w,
                                  std::u8string_view member) {
    auto data = dataLock(data_w);
    return data->getMemberRef(member);
}
FieldNameRef Field::getFieldRef(std::weak_ptr<Internal::ClientData> data_w,
                                std::u8string_view field) {
    auto data = dataLock(data_w);
    return data->getFieldRef(field);
}

Member Field::member() const {
    if (!member_) {
        throw std::invalid_argument("member name is null");
    }
    return *this;
}
std::string Field::name() const {
    if (!field_) {
        throw std::invalid_argument("field name is null");
    }
    return Encoding::getName(field_);
}
std::wstring Field::nameW() const {
    if (!field_) {
        throw std::invalid_argument("field name is null");
    }
    return Encoding::getNameW(field_);
}

bool Field::expired() const { return data_w.expired(); }

std::shared_ptr<Internal::ClientData>
Field::dataLock(std::weak_ptr<Internal::ClientData> data_w) {
    if (auto data = data_w.lock()) {
        return data;
    }
    throw std::runtime_error("Cannot access client data");
}

std::shared_ptr<Internal::ClientData> Field::setCheck() const {
    auto data = dataLock();
    if (!data->isSelf(*this)) {
        throw std::invalid_argument(
            "Cannot set data to member other than self");
    }
    return data;
}

bool Field::isSelf() const { return dataLock()->isSelf(*this); }

bool Field::operator==(const Field &other) const {
    return !expired() && !other.expired() &&
           data_w.lock().get() == other.data_w.lock().get() &&
           member_ == other.member_ && field_ == other.field_;
}


WEBCFACE_NS_END
