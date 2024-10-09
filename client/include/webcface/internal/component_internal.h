#pragma once
#include "webcface/component_canvas2d.h"
#include "webcface/component_canvas3d.h"
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN
namespace internal {

struct ViewComponentData : message::ViewComponent {
    ViewComponentData() = default;
    explicit ViewComponentData(const message::ViewComponent &vc)
        : message::ViewComponent(vc) {}

    // TemporalViewComponentとTemporalCanvas2DComponentの間でshareされるが
    // 同じfunctionが最終的に2つのcomponentに同時にsetされることはない
    std::shared_ptr<std::function<void()>> on_click_func_tmp;
    std::shared_ptr<std::function<void(ValAdaptor)>> on_change_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;

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
    explicit Canvas2DComponentData(const message::Canvas2DComponent &vc)
        : message::Canvas2DComponent(vc) {}

    std::shared_ptr<std::function<void()>> on_click_func_tmp;

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
