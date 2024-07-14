#include "c_wcf_internal.h"
#include "webcface/encoding/encoding.h"
#include "webcface/view.h"
#include <cstring>

static inline wcfViewComponent wcfViewInit() {
    wcfViewComponent c;
    std::memset(&c, 0, sizeof(c));
    c.min = -DBL_MAX;
    c.max = DBL_MAX;
    return c;
}

extern "C" {

wcfViewComponent wcfText(const char *text) {
    wcfViewComponent c = wcfViewInit();
    c.type = WCF_VIEW_TEXT;
    c.text = text;
    return c;
}
wcfViewComponent wcfNewLine() {
    wcfViewComponent c = wcfViewInit();
    c.type = WCF_VIEW_NEW_LINE;
    return c;
}
wcfViewComponent wcfButton(const char *text, const char *on_click_member,
                           const char *on_click_field) {
    wcfViewComponent c = wcfViewInit();
    c.type = WCF_VIEW_BUTTON;
    c.text = text;
    c.on_click_member = on_click_member;
    c.on_click_field = on_click_field;
    return c;
}
}

template <typename CharT>
static wcfStatus
wcfViewSetT(wcfClient *wcli, const CharT *field,
            const typename CharType<CharT>::CComponent *components, int size) {
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
        v.add(ViewComponent{
            static_cast<ViewComponentType>(p->type),
            SharedString::encode(strOrEmpty(p->text)),
            p->on_click_field
                ? std::make_optional<FieldBase>(
                      wcli_->member(strOrEmpty(p->on_click_member))
                          .child(strOrEmpty(p->on_click_field)))
                : std::nullopt,
            p->text_ref_field
                ? std::make_optional<FieldBase>(
                      wcli_->member(strOrEmpty(p->text_ref_member))
                          .child(strOrEmpty(p->text_ref_field)))
                : std::nullopt,
            static_cast<ViewColor>(p->text_color),
            static_cast<ViewColor>(p->bg_color),
            p->min != -DBL_MAX ? std::make_optional<double>(p->min)
                               : std::nullopt,
            p->max != DBL_MAX ? std::make_optional<double>(p->max)
                              : std::nullopt,
            p->step != 0 ? std::make_optional<double>(p->step) : std::nullopt,
            argsFromCVal<CharT>(p->option, p->option_num),
        });
    }
    v.sync();
    return WCF_OK;
}

template <typename CharT>
static wcfStatus
wcfViewGetT(wcfClient *wcli, const CharT *member, const CharT *field,
            typename CharType<CharT>::CComponent **components, int *recv_size) {
    *recv_size = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    auto vc = wcli_->member(strOrEmpty(member)).view(field).tryGet();
    if (vc) {
        if (!vc->empty()) {
            auto vcc_p = new typename CharType<CharT>::CComponent[vc->size()];
            *recv_size = static_cast<int>(vc->size());
            CharType<CharT>::viewList().emplace(vcc_p, std::move(*vc));
            auto &vc_ref = CharType<CharT>::viewList().at(vcc_p);
            for (std::size_t i = 0; i < vc_ref.size(); i++) {
                if constexpr (std::is_same_v<CharT, char>) {
                    vcc_p[i] = vc_ref[i].cData();
                } else {
                    vcc_p[i] = vc_ref[i].cDataW();
                }
            }
            *components = vcc_p;
        }
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}


extern "C" {
wcfStatus wcfViewSet(wcfClient *wcli, const char *field,
                     const wcfViewComponent *components, int size) {
    return wcfViewSetT(wcli, field, components, size);
}
wcfStatus wcfViewSetW(wcfClient *wcli, const wchar_t *field,
                      const wcfViewComponentW *components, int size) {
    return wcfViewSetT(wcli, field, components, size);
}
wcfStatus wcfViewGet(wcfClient *wcli, const char *member, const char *field,
                     wcfViewComponent **components, int *recv_size) {
    return wcfViewGetT(wcli, member, field, components, recv_size);
}
wcfStatus wcfViewGetW(wcfClient *wcli, const wchar_t *member,
                      const wchar_t *field, wcfViewComponentW **components,
                      int *recv_size) {
    return wcfViewGetT(wcli, member, field, components, recv_size);
}
}
