#pragma once
#include <string>
#include <optional>
#include <vector>
#include "../field.h"
#include <webcface/common/def.h>
#include "../val_adaptor.h"

WEBCFACE_NS_BEGIN
inline namespace Common {
enum class ViewComponentType {
    text = 0,
    new_line = 1,
    button = 2,
    text_input = 3,
    decimal_input = 4,
    number_input = 5,
    toggle_input = 6,
    select_input = 7,
    slider_input = 8,
    check_input = 9,
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
    SharedString text_;
    std::optional<FieldBase> on_click_func_;
    std::optional<FieldBase> text_ref_;
    ViewColor text_color_ = ViewColor::inherit;
    ViewColor bg_color_ = ViewColor::inherit;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt,
                          step_ = std::nullopt;
    std::vector<ValAdaptor> option_ = {};

    bool operator==(const ViewComponentBase &rhs) const {
        return type_ == rhs.type_ && text_ == rhs.text_ &&
               on_click_func_ == rhs.on_click_func_ &&
               text_ref_ == rhs.text_ref_ && text_color_ == rhs.text_color_ &&
               bg_color_ == rhs.bg_color_ && min_ == rhs.min_ &&
               max_ == rhs.max_ && step_ == rhs.step_ && option_ == rhs.option_;
    }
    bool operator!=(const ViewComponentBase &rhs) const {
        return !(*this == rhs);
    }
};

} // namespace Common
WEBCFACE_NS_END
