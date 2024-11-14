#pragma once
#include "webcface/common/internal/message/view.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/text.h"
#include <functional>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace internal {

struct TemporalViewComponentData : message::ViewComponentData {
    TemporalViewComponentData() = default;

    // TemporalViewComponentとTemporalCanvas2DComponentの間でshareされるが
    // 同じfunctionが最終的に2つのcomponentに同時にsetされることはない
    std::shared_ptr<std::function<void()>> on_click_func_tmp;
    std::shared_ptr<std::function<void(ValAdaptor)>> on_change_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;
    SharedString id;
};

struct TemporalCanvas2DComponentData : message::Canvas2DComponentData {
    TemporalCanvas2DComponentData() = default;

    std::shared_ptr<std::function<void()>> on_click_func_tmp;
    SharedString id;
};

struct TemporalCanvas3DComponentData : message::Canvas3DComponentData {
    TemporalCanvas3DComponentData() = default;

    std::weak_ptr<internal::ClientData> data_w;
    SharedString id;

    auto &anglesAt(std::size_t i) { return angles[std::to_string(i)]; }
};

} // namespace internal
WEBCFACE_NS_END
