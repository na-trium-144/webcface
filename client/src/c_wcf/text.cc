#include "c_wcf_internal.h"
#include "webcface/text.h"
#include <cstring>

template <typename CharT>
static wcfStatus wcfTextSetT(wcfClient *wcli, const CharT *field,
                             const CharT *text) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    wcli_->text(field).set(strOrEmpty(text));
    return WCF_OK;
}
template <typename CharT>
static wcfStatus wcfTextSetNT(wcfClient *wcli, const CharT *field,
                              const CharT *text, int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    wcli_->text(field).set(std::basic_string_view<CharT>(text, size));
    return WCF_OK;
}

template <typename CharT>
static wcfStatus wcfTextGetT(wcfClient *wcli, const CharT *member,
                             const CharT *field, CharT *text, int size,
                             int *recv_size) {
    *recv_size = 0;
    if (!field || size <= 0) {
        return WCF_INVALID_ARGUMENT;
    }
    text[0] = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto str = wcli_->member(strOrEmpty(member)).text(field).tryGetV();
    if (str) {
        const std::basic_string<CharT> &str2 = *str;
        int copy_size = (size - 1) < static_cast<int>(str2.size())
                            ? (size - 1)
                            : static_cast<int>(str2.size());
        std::memcpy(text, str2.c_str(), copy_size * sizeof(CharT));
        text[copy_size] = 0;
        *recv_size = static_cast<int>(str2.size());
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}
extern "C" {
wcfStatus wcfTextSet(wcfClient *wcli, const char *field, const char *text) {
    return wcfTextSetT(wcli, field, text);
}
wcfStatus wcfTextSetW(wcfClient *wcli, const wchar_t *field,
                      const wchar_t *text) {
    return wcfTextSetT(wcli, field, text);
}
wcfStatus wcfTextSetN(wcfClient *wcli, const char *field, const char *text,
                      int size) {
    return wcfTextSetNT(wcli, field, text, size);
}
wcfStatus wcfTextSetNW(wcfClient *wcli, const wchar_t *field,
                       const wchar_t *text, int size) {
    return wcfTextSetNT(wcli, field, text, size);
}
wcfStatus wcfTextGet(wcfClient *wcli, const char *member, const char *field,
                     char *text, int size, int *recv_size) {
    return wcfTextGetT(wcli, member, field, text, size, recv_size);
}
wcfStatus wcfTextGetW(wcfClient *wcli, const wchar_t *member,
                      const wchar_t *field, wchar_t *text, int size,
                      int *recv_size) {
    return wcfTextGetT(wcli, member, field, text, size, recv_size);
}
}
