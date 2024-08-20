#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
namespace internal{
struct ViewComponentData {
    std::weak_ptr<internal::ClientData> data_w;
    int idx_for_type;

    ViewComponentType type_ = ViewComponentType::text;
    SharedString text_;
    std::optional<FieldBase> on_click_func_;
    std::optional<FieldBase> text_ref_;
    ViewColor text_color_ = ViewColor::inherit;
    ViewColor bg_color_ = ViewColor::inherit;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt,
                          step_ = std::nullopt;
    std::vector<ValAdaptor> option_ = {};

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;

    // for cData()
    mutable std::vector<wcfMultiVal> options_s;
    mutable std::vector<wcfMultiValW> options_sw;

    template <typename CComponent, typename CVal, std::size_t v_index>
    CComponent cDataT() const;
};

}
WEBCFACE_NS_END
