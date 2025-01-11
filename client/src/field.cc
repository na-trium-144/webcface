#include "webcface/field.h"
#include "webcface/member.h"
#include "webcface/value.h"
#include "webcface/text.h"
#include "webcface/robot_model.h"
#include "webcface/image.h"
#include "webcface/view.h"
#include "webcface/func.h"
#include "webcface/canvas2d.h"
#include "webcface/canvas3d.h"
#include "webcface/log.h"
#include "webcface/internal/client_internal.h"
#include <stdexcept>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
SharedString Field::lastName8() const {
    auto i = this->field_.u8String().rfind(field_separator);
    if (i != std::string::npos && i != 0 &&
        !(i == 1 && this->field_.u8String()[0] == field_separator)) {
        return SharedString::fromU8String(
            this->field_.u8StringView().substr(i + 1));
    } else {
        return this->field_;
    }
}

Field Field::parent() const {
    int l = static_cast<int>(this->field_.u8String().size()) -
            static_cast<int>(lastName8().u8String().size()) - 1;
    if (l < 0) {
        l = 0;
    }
    return Field{*this, SharedString::fromU8String(
                            this->field_.u8StringView().substr(0, l))};
}
Field Field::child(const SharedString &field) const {
    if (this->field_.empty()) {
        return Field{*this, field};
    } else if (field.empty()) {
        return *this;
    } else {
        return Field{*this, SharedString::fromU8String(this->field_.u8String() +
                                                       field_separator +
                                                       field.u8String())};
    }
}

/// \private
template <typename V, typename S>
static auto entries(const Field *this_, S &store) {
    auto keys = store.getEntry(*this_);
    std::vector<V> ret;
    for (const auto &f : keys) {
        if (this_->field_.empty() ||
            f.startsWith(this_->field_.u8String() + field_separator)) {
            ret.emplace_back(this_->child(f));
        }
    }
    return ret;
}
template <typename T, std::nullptr_t>
std::vector<T> Field::valueEntries() const {
    return entries<Value>(this, dataLock()->value_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::textEntries() const {
    return entries<Text>(this, dataLock()->text_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::robotModelEntries() const {
    return entries<RobotModel>(this, dataLock()->robot_model_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::funcEntries() const {
    return entries<Func>(this, dataLock()->func_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::viewEntries() const {
    return entries<View>(this, dataLock()->view_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::canvas3DEntries() const {
    return entries<Canvas3D>(this, dataLock()->canvas3d_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::canvas2DEntries() const {
    return entries<Canvas2D>(this, dataLock()->canvas2d_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::imageEntries() const {
    return entries<Image>(this, dataLock()->image_store);
}
template <typename T, std::nullptr_t>
std::vector<T> Field::logEntries() const {
    return entries<Log>(this, dataLock()->log_store);
}

template WEBCFACE_DLL std::vector<Value>
Field::valueEntries<Value, nullptr>() const;
template WEBCFACE_DLL std::vector<Text>
Field::textEntries<Text, nullptr>() const;
template WEBCFACE_DLL std::vector<RobotModel>
Field::robotModelEntries<RobotModel, nullptr>() const;
template WEBCFACE_DLL std::vector<Func>
Field::funcEntries<Func, nullptr>() const;
template WEBCFACE_DLL std::vector<View>
Field::viewEntries<View, nullptr>() const;
template WEBCFACE_DLL std::vector<Canvas2D>
Field::canvas2DEntries<Canvas2D, nullptr>() const;
template WEBCFACE_DLL std::vector<Canvas3D>
Field::canvas3DEntries<Canvas3D, nullptr>() const;
template WEBCFACE_DLL std::vector<Image>
Field::imageEntries<Image, nullptr>() const;
template WEBCFACE_DLL std::vector<Log> Field::logEntries<Log, nullptr>() const;

bool Field::expired() const { return data_w.expired(); }

std::shared_ptr<internal::ClientData> Field::dataLock() const {
    if (auto data = data_w.lock()) {
        return data;
    }
    throw std::runtime_error("Cannot access client data");
}

std::shared_ptr<internal::ClientData> Field::setCheck() const {
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
