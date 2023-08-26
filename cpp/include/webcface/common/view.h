#pragma once
#include <string>
#include "field_base.h"

namespace WebCFace {
inline namespace Common {
enum class ViewComponentType {
    text = 0,
    new_line = 1,
    button = 2,
};
class ViewComponent {
  protected:
    ViewComponentType type_;
    std::string text_;
    FieldBase on_click_func_;

  public:
    ViewComponent() = default;
    explicit ViewComponent(ViewComponentType type) : type_(type) {}
    ViewComponent(const std::string &text)
        : type_(ViewComponentType::text), text_(text) {}
    bool operator==(const ViewComponent &rhs) const {
        return type_ == rhs.type_ && text_ == rhs.text_;
    }
    bool operator!=(const ViewComponent &rhs) const { return !(*this == rhs); }

    ViewComponentType type() const { return type_; }
    std::string text() const { return text_; }
};

inline namespace ViewComponents {
inline ViewComponent newLine() {
    return ViewComponent(ViewComponentType::new_line);
}
} // namespace ViewComponents

inline std::unordered_map<int, ViewComponent>
getViewDiff(const std::vector<ViewComponent> &current,
            const std::vector<ViewComponent> &prev) {
    std::unordered_map<int, ViewComponent> diff;
    for (std::size_t i = 0; i < current.size(); i++) {
        if (prev.size() <= i || prev[i] != current[i]) {
            diff[i] = current[i];
        }
    }
    return diff;
}
template <typename T>
void mergeViewDiff(const std::unordered_map<int, T> &diff, int size,
                   std::vector<ViewComponent> &prev) {
    prev.resize(size);
    for (const auto &v : diff) {
        prev[v.first] = static_cast<ViewComponent>(v.second);
    }
}
} // namespace Common
} // namespace WebCFace