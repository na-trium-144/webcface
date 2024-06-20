#include <webcface/component_canvas2d.h>
#include <webcface/member.h>
#include <webcface/robot_model.h>
#include "webcface/message/message.h"
#include "client_internal.h"

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

Message::Canvas2DComponent Canvas2DComponent::toMessage() const {
    Message::Canvas2DComponent cc;
    cc.type = static_cast<int>(this->type_);
    cc.origin_pos = {this->origin_.pos(0), this->origin_.pos(1)};
    cc.origin_rot = this->origin_.rot(0);
    cc.color = static_cast<int>(this->color_);
    cc.fill = static_cast<int>(this->fill_);
    cc.stroke_width = this->stroke_width_;
    cc.text = this->text_;
    if (this->geometry_) {
        cc.geometry_type = static_cast<int>(this->geometry_->type);
        cc.properties = this->geometry_->properties;
    }
    if (this->on_click_func_) {
        cc.on_click_member = this->on_click_func_->member_;
        cc.on_click_field = this->on_click_func_->field_;
    }
    return cc;
}
Canvas2DComponent::Canvas2DComponent(const Message::Canvas2DComponent &cc)
    : Canvas2DComponent(
          static_cast<Canvas2DComponentType>(cc.type),
          {cc.origin_pos, cc.origin_rot}, static_cast<ViewColor>(cc.color),
          static_cast<ViewColor>(cc.fill), cc.stroke_width,
          std::make_optional<Geometry>(
              static_cast<GeometryType>(cc.geometry_type), cc.properties),
          (cc.on_click_member && cc.on_click_field
               ? std::make_optional<FieldBase>(*cc.on_click_member,
                                               *cc.on_click_field)
               : std::nullopt),
          cc.text) {}

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
