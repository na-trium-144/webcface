#include <webcface/field.h>
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/robot_model.h>
#include <webcface/image.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/canvas2d.h>
#include <webcface/canvas3d.h>
#include "client_internal.h"
#include <stdexcept>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
Member Field::member() const { return *this; }

Value Field::value(std::string_view field) const { return child(field); }
Text Field::text(std::string_view field) const { return child(field); }
RobotModel Field::robotModel(std::string_view field) const { return child(field); }
Image Field::image(std::string_view field) const { return child(field); }
Func Field::func(std::string_view field) const { return child(field); }
View Field::view(std::string_view field) const { return child(field); }
Canvas3D Field::canvas3D(std::string_view field) const { return child(field); }
Canvas2D Field::canvas2D(std::string_view field) const { return child(field); }

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
