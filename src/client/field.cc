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
std::u8string_view Field::memberRef() const {
    if (!member_) {
        throw std::invalid_argument("member name is null");
    }
    return Encoding::getNameU8(member_);
}
std::u8string_view Field::nameRef() const {
    if (!field_) {
        throw std::invalid_argument("field name is null");
    }
    return Encoding::getNameU8(field_);
}
std::u8string_view Field::lastNameRef() const {
    auto i = nameRef().rfind(field_separator);
    if (i != std::string::npos && i != 0 &&
        !(i == 1 && nameRef()[0] == field_separator)) {
        return nameRef().substr(i + 1);
    } else {
        return nameRef();
    }
}
Field Field::parent() const {
    int l = nameRef().size() - lastName().size() - 1;
    if (l < 0) {
        l = 0;
    }
    return Field{*this, nameRef().substr(0, l)};
}
Field Field::child(std::u8string_view field) const {
    if (nameRef().empty()) {
        return Field{*this, field};
    } else if (field.empty()) {
        return *this;
    } else {
        return Field{*this,
                     std::u8string(nameRef()) + field_separator + std::u8string(field)};
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
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<Text> Field::textEntries() const {
    auto keys = dataLock()->text_store.getEntry(*this);
    std::vector<Text> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<RobotModel> Field::robotModelEntries() const {
    auto keys = dataLock()->robot_model_store.getEntry(*this);
    std::vector<RobotModel> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<Func> Field::funcEntries() const {
    auto keys = dataLock()->func_store.getEntry(*this);
    std::vector<Func> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<View> Field::viewEntries() const {
    auto keys = dataLock()->view_store.getEntry(*this);
    std::vector<View> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<Canvas3D> Field::canvas3DEntries() const {
    auto keys = dataLock()->canvas3d_store.getEntry(*this);
    std::vector<Canvas3D> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<Canvas2D> Field::canvas2DEntries() const {
    auto keys = dataLock()->canvas2d_store.getEntry(*this);
    std::vector<Canvas2D> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
}
std::vector<Image> Field::imageEntries() const {
    auto keys = dataLock()->image_store.getEntry(*this);
    std::vector<Image> ret;
    for (const auto &fp : keys) {
        auto f = Encoding::getNameU8(fp);
        if (nameRef().empty() ||
            (f.starts_with(nameRef()) && f[nameRef().size()] == u8'.')) {
            ret.emplace_back(*this, f);
        }
    }
    return ret;
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
