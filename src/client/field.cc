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
RobotModel Field::robotModel(std::string_view field) const {
    return child(field);
}
Image Field::image(std::string_view field) const { return child(field); }
Func Field::func(std::string_view field) const { return child(field); }
View Field::view(std::string_view field) const { return child(field); }
Canvas3D Field::canvas3D(std::string_view field) const { return child(field); }
Canvas2D Field::canvas2D(std::string_view field) const { return child(field); }

std::vector<Value> Field::valueEntries() const {
    auto keys = dataLock()->value_store.getEntry(*this);
    std::vector<Value> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = value(keys[i]);
    }
    return ret;
}
std::vector<Text> Field::textEntries() const {
    auto keys = dataLock()->text_store.getEntry(*this);
    std::vector<Text> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = text(keys[i]);
    }
    return ret;
}
std::vector<RobotModel> Field::robotModelEntries() const {
    auto keys = dataLock()->robot_model_store.getEntry(*this);
    std::vector<RobotModel> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = robotModel(keys[i]);
    }
    return ret;
}
std::vector<Func> Field::funcEntries() const {
    auto keys = dataLock()->func_store.getEntry(*this);
    std::vector<Func> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = func(keys[i]);
    }
    return ret;
}
std::vector<View> Field::viewEntries() const {
    auto keys = dataLock()->view_store.getEntry(*this);
    std::vector<View> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = view(keys[i]);
    }
    return ret;
}
std::vector<Canvas3D> Field::canvas3DEntries() const {
    auto keys = dataLock()->canvas3d_store.getEntry(*this);
    std::vector<Canvas3D> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = canvas3D(keys[i]);
    }
    return ret;
}
std::vector<Canvas2D> Field::canvas2DEntries() const {
    auto keys = dataLock()->canvas2d_store.getEntry(*this);
    std::vector<Canvas2D> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = canvas2D(keys[i]);
    }
    return ret;
}
std::vector<Image> Field::imageEntries() const {
    auto keys = dataLock()->image_store.getEntry(*this);
    std::vector<Image> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = image(keys[i]);
    }
    return ret;
}

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
