#include "c_wcf_internal.h"

extern "C" {
wcfStatus wcfTextSet(wcfClient *wcli, const char *field, const char *text) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->text(field).set(text);
    return WCF_OK;
}
wcfStatus wcfTextSetN(wcfClient *wcli, const char *field, const char *text,
                      int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->text(field).set(std::string(text, size));
    return WCF_OK;
}
wcfStatus wcfTextGet(wcfClient *wcli, const char *member, const char *field,
                     char *text, int size, int *recv_size) {
    *recv_size = 0;
    if (size <= 0) {
        return WCF_INVALID_ARGUMENT;
    }
    text[0] = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto str = wcli_->member(member).text(field).tryGet();
    if (str) {
        int copy_size = (size - 1) < static_cast<int>(str->size())
                            ? (size - 1)
                            : str->size();
        std::memcpy(text, str->c_str(), copy_size * sizeof(char));
        text[copy_size] = 0;
        *recv_size = str->size();
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}
}