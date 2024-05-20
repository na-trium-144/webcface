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
std::u8string_view Field::lastName8() const {
    auto i = this->field_.rfind(field_separator);
    if (i != std::string::npos && i != 0 &&
        !(i == 1 && this->field_[0] == field_separator)) {
        return std::u8string_view(this->field_).substr(i + 1);
    } else {
        return this->field_;
    }
}

Field Field::parent() const {
    int l = static_cast<int>(this->field_.size()) -
            static_cast<int>(lastName().size()) - 1;
    if (l < 0) {
        l = 0;
    }
    return Field{*this, this->field_.substr(0, l)};
}
Field Field::child(std::u8string_view field) const {
    if (this->field_.empty()) {
        return Field{*this, field};
    } else if (field.empty()) {
        return *this;
    } else {
        return Field{*this,
                     this->field_ + field_separator + std::u8string(field)};
    }
}

Value Field::value(std::string_view field) const { return child(field); }
Value Field::value(std::wstring_view field) const { return child(field); }
Text Field::text(std::string_view field) const { return child(field); }
Text Field::text(std::wstring_view field) const { return child(field); }
RobotModel Field::robotModel(std::string_view field) const {
    return child(field);
}
RobotModel Field::robotModel(std::wstring_view field) const {
    return child(field);
}
Image Field::image(std::string_view field) const { return child(field); }
Image Field::image(std::wstring_view field) const { return child(field); }
Func Field::func(std::string_view field) const { return child(field); }
Func Field::func(std::wstring_view field) const { return child(field); }
View Field::view(std::string_view field) const { return child(field); }
View Field::view(std::wstring_view field) const { return child(field); }
Canvas3D Field::canvas3D(std::string_view field) const { return child(field); }
Canvas3D Field::canvas3D(std::wstring_view field) const { return child(field); }
Canvas2D Field::canvas2D(std::string_view field) const { return child(field); }
Canvas2D Field::canvas2D(std::wstring_view field) const { return child(field); }

std::vector<Value> Field::valueEntries() const {
    auto keys = dataLock()->value_store.getEntry(*this);
    std::vector<Value> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(value(f));
        }
    }
    return ret;
}
std::vector<Text> Field::textEntries() const {
    auto keys = dataLock()->text_store.getEntry(*this);
    std::vector<Text> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(text(f));
        }
    }
    return ret;
}
std::vector<RobotModel> Field::robotModelEntries() const {
    auto keys = dataLock()->robot_model_store.getEntry(*this);
    std::vector<RobotModel> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(robotModel(f));
        }
    }
    return ret;
}
std::vector<Func> Field::funcEntries() const {
    auto keys = dataLock()->func_store.getEntry(*this);
    std::vector<Func> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(func(f));
        }
    }
    return ret;
}
std::vector<View> Field::viewEntries() const {
    auto keys = dataLock()->view_store.getEntry(*this);
    std::vector<View> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(view(f));
        }
    }
    return ret;
}
std::vector<Canvas3D> Field::canvas3DEntries() const {
    auto keys = dataLock()->canvas3d_store.getEntry(*this);
    std::vector<Canvas3D> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(canvas3D(f));
        }
    }
    return ret;
}
std::vector<Canvas2D> Field::canvas2DEntries() const {
    auto keys = dataLock()->canvas2d_store.getEntry(*this);
    std::vector<Canvas2D> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(canvas2D(f));
        }
    }
    return ret;
}
std::vector<Image> Field::imageEntries() const {
    auto keys = dataLock()->image_store.getEntry(*this);
    std::vector<Image> ret;
    for (const auto &f : keys) {
        if (this->field_.empty() ||
            f.starts_with(this->field_ + field_separator)) {
            ret.push_back(image(f));
        }
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
