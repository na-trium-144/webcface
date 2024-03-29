#pragma once
#include <string>
#include <optional>
#include "field_base.h"
#include "def.h"

namespace WEBCFACE_NS {
inline namespace Common {
enum class ViewComponentType {
    text = 0,
    new_line = 1,
    button = 2,
};
enum class ViewColor {
    inherit = 0,
    black = 1,
    white = 2,
    // slate = 3,
    gray = 4,
    // zinc = 5,
    // neutral = 6,
    // stone = 7,
    red = 8,
    orange = 9,
    // amber = 10,
    yellow = 11,
    // lime = 12,
    green = 13,
    // emerald = 14,
    teal = 15,
    cyan = 16,
    // sky = 17,
    blue = 18,
    indigo = 19,
    // violet = 20,
    purple = 21,
    // fuchsia = 22,
    pink = 23,
    // rose = 24,
};
struct ViewComponentBase {
    ViewComponentType type_ = ViewComponentType::text;
    std::string text_;
    std::optional<FieldBase> on_click_func_;
    ViewColor text_color_ = ViewColor::inherit;
    ViewColor bg_color_ = ViewColor::inherit;

    bool operator==(const ViewComponentBase &rhs) const {
        return type_ == rhs.type_ && text_ == rhs.text_ &&
               ((on_click_func_ == std::nullopt &&
                 rhs.on_click_func_ == std::nullopt) ||
                (on_click_func_ && rhs.on_click_func_ &&
                 on_click_func_->member_ == rhs.on_click_func_->member_ &&
                 on_click_func_->field_ == rhs.on_click_func_->field_)) &&
               text_color_ == rhs.text_color_ && bg_color_ == rhs.bg_color_;
    }
    bool operator!=(const ViewComponentBase &rhs) const {
        return !(*this == rhs);
    }
};

} // namespace Common
} // namespace WEBCFACE_NS
