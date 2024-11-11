#pragma once
#include "webcface/component_canvas2d.h"
#include "webcface/component_canvas3d.h"
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN
namespace internal {

struct ViewComponentData : message::ViewComponent {
    ViewComponentData() = default;
    explicit ViewComponentData(const message::ViewComponent &vc,
                               const SharedString &id)
        : message::ViewComponent(vc), id(id) {}

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;
    SharedString id;

    // for cData()
    mutable std::vector<wcfMultiVal> options_s;
    mutable std::vector<wcfMultiValW> options_sw;

    template <typename CComponent, typename CVal, std::size_t v_index>
    CComponent cDataT() const;

    bool operator==(const ViewComponentData &other) const;
    bool operator!=(const ViewComponentData &other) const {
        return !(*this == other);
    }
};

struct Canvas2DComponentData : message::Canvas2DComponent {
    Canvas2DComponentData() = default;
    explicit Canvas2DComponentData(const message::Canvas2DComponent &vc,
                                   const SharedString &id)
        : message::Canvas2DComponent(vc), id(id) {}

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    SharedString id;

    bool operator==(const Canvas2DComponentData &other) const;
    bool operator!=(const Canvas2DComponentData &other) const {
        return !(*this == other);
    }
};

struct Canvas3DComponentData : message::Canvas3DComponent {
    Canvas3DComponentData() = default;
    explicit Canvas3DComponentData(const message::Canvas3DComponent &vc)
        : message::Canvas3DComponent(vc) {}

    std::weak_ptr<internal::ClientData> data_w;

    auto &anglesAt(std::size_t i) { return angles[std::to_string(i)]; }

    bool operator==(const Canvas3DComponentData &other) const;
    bool operator!=(const Canvas3DComponentData &other) const {
        return !(*this == other);
    }
};

} // namespace internal
WEBCFACE_NS_END
