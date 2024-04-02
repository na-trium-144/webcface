#include "c_wcf_internal.h"
#include <webcface/view.h>

extern "C" {

wcfViewComponent wcfText(const char *text) {
    return wcfViewComponent{
        .type = WCF_VIEW_TEXT,
        .text = text,
        .on_click_member = nullptr,
        .on_click_field = nullptr,
        .text_ref_member = nullptr,
        .text_ref_field = nullptr,
        .text_color = WCF_COLOR_INHERIT,
        .bg_color = WCF_COLOR_INHERIT,
    };
}
wcfViewComponent wcfNewLine() {
    return wcfViewComponent{
        .type = WCF_VIEW_NEW_LINE,
        .text = nullptr,
        .on_click_member = nullptr,
        .on_click_field = nullptr,
        .text_ref_member = nullptr,
        .text_ref_field = nullptr,
        .text_color = WCF_COLOR_INHERIT,
        .bg_color = WCF_COLOR_INHERIT,
    };
}
wcfViewComponent wcfButton(const char *text, const char *on_click_member,
                           const char *on_click_field) {
    return wcfViewComponent{
        .type = WCF_VIEW_BUTTON,
        .text = text,
        .on_click_member = on_click_member,
        .on_click_field = on_click_field,
        .text_ref_member = nullptr,
        .text_ref_field = nullptr,
        .text_color = WCF_COLOR_INHERIT,
        .bg_color = WCF_COLOR_INHERIT,
    };
}

wcfStatus wcfViewSet(wcfClient *wcli, const char *field,
                     const wcfViewComponent *components, int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    auto v = wcli_->view(field);
    v.init();
    for (auto p = components; p < components + size; p++) {
        v.add(ViewComponentBase{
            static_cast<ViewComponentType>(p->type),
            p->text ? p->text : "",
            p->on_click_field
                ? std::make_optional<FieldBase>(
                      p->on_click_member ? p->on_click_member : wcli_->name(),
                      p->on_click_field ? p->on_click_field : "")
                : std::nullopt,
            p->text_ref_field
                ? std::make_optional<FieldBase>(
                      p->text_ref_member ? p->text_ref_member : wcli_->name(),
                      p->text_ref_field ? p->text_ref_field : "")
                : std::nullopt,
            static_cast<ViewColor>(p->text_color),
            static_cast<ViewColor>(p->bg_color),
            // todo
        });
    }
    v.sync();
    return WCF_OK;
}

wcfStatus wcfViewGet(wcfClient *wcli, const char *member, const char *field,
                     wcfViewComponent **components, int *recv_size) {
    *recv_size = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    auto vc = wcli_->member(member ? member : "").view(field).tryGet();
    if (vc) {
        if (!vc->empty()) {
            auto vcc_p = new wcfViewComponent[vc->size()];
            *recv_size = vc->size();
            view_list.emplace(vcc_p, std::move(*vc));
            auto &vc_ref = view_list.at(vcc_p);
            for (std::size_t i = 0; i < vc_ref.size(); i++) {
                vcc_p[i] = vc_ref[i].cData();
            }
            *components = vcc_p;
        }
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}
}
