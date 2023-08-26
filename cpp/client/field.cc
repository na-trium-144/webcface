#include <webcface/field.h>
#include <webcface/member.h>
#include <webcface/client_data.h>
#include <stdexcept>
#include <cassert>

namespace WebCFace {
Member Field::member() const { return *this; }

std::shared_ptr<ClientData> Field::dataLock() const {
    if (auto data = data_w.lock()) {
        return data;
    }
    throw std::runtime_error("Cannot access client data");
}

void Field::setCheck() const {
    assert(dataLock()->isSelf(*this) &&
           "Cannot set data to member other than self");
}
} // namespace WebCFace