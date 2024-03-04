#include <webcface/canvas_data.h>
#include <webcface/member.h>
#include "client_internal.h"

namespace WEBCFACE_NS {

Canvas2DComponentBase &
Canvas2DComponent::lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
                           const std::string &field_id) {
    if (on_click_func_tmp != nullptr) {
        auto data = data_w.lock();
        Func on_click{Field{data_w, data->self_member_name}, field_id};
        on_click_func_tmp->lockTo(on_click);
        on_click.hidden(true);
        onClick(on_click);
    }
    if (common_geometry_tmp != nullptr) {
        geometry_ = std::make_optional<Geometry>(*common_geometry_tmp);
    }
    return *this;
}

std::optional<Func> Canvas2DComponent::onClick() const {
    if (on_click_func_ != std::nullopt) {
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
Canvas2DComponent &Canvas2DComponent::onClick(const Func &func) {
    on_click_func_ = FieldBase{func.member().name(), func.name()};
    return *this;
}


} // namespace WEBCFACE_NS
