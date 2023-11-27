#include <webcface/field.h>
#include <webcface/member.h>
#include "client_internal.h"
#include <stdexcept>

namespace webcface {
Member Field::member() const { return *this; }

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
} // namespace webcface