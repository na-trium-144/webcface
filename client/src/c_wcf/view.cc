#include "c_wcf_internal.h"
#include "webcface/common/encoding.h"
#include "webcface/view.h"
#include <cstring>

// 変更するとABI互換性がなくなる
#if WEBCFACE_PTR_SIZE == 4
static_assert(sizeof(wcfViewComponent) == 4L * 24,
              "sizeof wcfViewCompoenent mismatch");
static_assert(sizeof(wcfViewComponentW) == 4L * 24,
              "sizeof wcfViewCompoenentW mismatch");
#elif WEBCFACE_PTR_SIZE == 8
static_assert(sizeof(wcfViewComponent) == 4L * 36,
              "sizeof wcfViewCompoenent mismatch");
static_assert(sizeof(wcfViewComponentW) == 4L * 36,
              "sizeof wcfViewCompoenentW mismatch");
#else
#pragma message("warning: sizeof void* is neither 4 or 8")
#endif

/// \private
template <typename CharT>
static auto wcfViewInit() {
    typename CharType<CharT>::CComponent c;
    std::memset(&c, 0, sizeof(c));
    c.min = -DBL_MAX;
    c.max = DBL_MAX;
    return c;
}
/// \private
template <typename CharT>
static auto wcfTextT(const CharT *text) {
    auto c = wcfViewInit<CharT>();
    c.type = WCF_VIEW_TEXT;
    c.text = text;
    return c;
}
/// \private
template <typename CharT>
static auto wcfNewLineT() {
    auto c = wcfViewInit<CharT>();
    c.type = WCF_VIEW_NEW_LINE;
    return c;
}
/// \private
template <typename CharT>
static auto wcfButtonT(const CharT *text, const CharT *on_click_member,
                       const CharT *on_click_field) {
    auto c = wcfViewInit<CharT>();
    c.type = WCF_VIEW_BUTTON;
    c.text = text;
    c.on_click_member = on_click_member;
    c.on_click_field = on_click_field;
    return c;
}

extern "C" {

wcfViewComponent wcfText(const char *text) { return wcfTextT(text); }
wcfViewComponentW wcfTextW(const wchar_t *text) { return wcfTextT(text); }
wcfViewComponent wcfNewLine() { return wcfNewLineT<char>(); }
wcfViewComponentW wcfNewLineW() { return wcfNewLineT<wchar_t>(); }
wcfViewComponent wcfButton(const char *text, const char *on_click_member,
                           const char *on_click_field) {
    return wcfButtonT(text, on_click_member, on_click_field);
}
wcfViewComponentW wcfButtonW(const wchar_t *text,
                             const wchar_t *on_click_member,
                             const wchar_t *on_click_field) {
    return wcfButtonT(text, on_click_member, on_click_field);
}
}

/// \private
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
        TemporalViewComponent cp{static_cast<ViewComponentType>(p->type)};
        cp.text(strOrEmpty(p->text));
        if (p->on_click_field) {
            cp.onClick(wcli_->member(strOrEmpty(p->on_click_member))
                           .child(strOrEmpty(p->on_click_field))
                           .func());
        }
        /*if(p->text_ref_field){
            cp.bind(wcli_->member(strOrEmpty(p->text_ref_member))
                          .child(strOrEmpty(p->text_ref_field)));
        }*/
        cp.textColor(static_cast<ViewColor>(p->text_color));
        cp.bgColor(static_cast<ViewColor>(p->bg_color));
        if (p->min != -DBL_MAX) {
            cp.min(p->min);
        }
        if (p->max != DBL_MAX) {
            cp.max(p->max);
        }
        if (p->step != 0) {
            cp.step(p->step);
        }
        cp.option(argsFromCVal<CharT>(p->option, p->option_num));
        v.add(std::move(cp));
    }
    v.sync();
    return WCF_OK;
}

/// \private
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
        return WCF_NO_DATA;
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
