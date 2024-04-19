#include <webcface/field.h>
#include <webcface/member.h>
#include "client_internal.h"
#include <stdexcept>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
Field::Field(const std::weak_ptr<Internal::ClientData> &data_w,
             std::string_view member, std::string_view field)
    : Common::FieldBase(), data_w(data_w) {
    auto data = dataLock();
    member_ = data->getMemberRef(member);
    field_ = data->getFieldRef(field);
}
Field::Field(const std::weak_ptr<Internal::ClientData> &data_w,
             std::string_view member)
    : Common::FieldBase(), data_w(data_w) {
    auto data = dataLock();
    member_ = data->getMemberRef(member);
}

Member Field::member() const { return *this; }

bool Field::expired() const { return data_w.expired(); }

std::shared_ptr<Internal::ClientData> Field::dataLock() const {
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
           static_cast<FieldBase>(*this) == static_cast<FieldBase>(other);
}


WEBCFACE_NS_END
