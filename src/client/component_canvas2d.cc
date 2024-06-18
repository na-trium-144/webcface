#include <webcface/component_canvas2d.h>
#include <webcface/member.h>
#include <webcface/robot_model.h>

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF IdBase<Canvas2DComponentType>;

Canvas2DComponent &
Canvas2DComponent::lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
                           const SharedString &view_name,
                           std::unordered_map<int, int> *idx_next) {
    initIdx(idx_next, type_);
    if (on_click_func_tmp != nullptr) {
        auto data = data_w.lock();
        Func on_click{Field{data_w, data->self_member_name},
                      SharedString(u8"..c2" + view_name.u8String() + u8"/" +
                                   std::u8string(Encoding::castToU8(id())))};
        on_click_func_tmp->lockTo(on_click);
        onClick(on_click);
    }
    return *this;
}

std::optional<Func> Canvas2DComponent::onClick() const {
    if (on_click_func_) {
        return Field{data_w, on_click_func_->member_, on_click_func_->field_};
    } else {
        return std::nullopt;
    }
}
Canvas2DComponent &Canvas2DComponent::onClick(const Func &func) {
    on_click_func_.emplace(static_cast<FieldBase>(func));
    return *this;
}


WEBCFACE_NS_END
