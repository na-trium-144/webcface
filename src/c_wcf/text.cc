#include "c_wcf_internal.h"
#include <webcface/text.h>
#include <cstring>

extern "C" {
wcfStatus wcfTextSet(wcfClient *wcli, const char *field, const char *text) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    wcli_->text(field).set(text ? text : "");
    return WCF_OK;
}
wcfStatus wcfTextSetN(wcfClient *wcli, const char *field, const char *text,
                      int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    wcli_->text(field).set(std::string(text, size));
    return WCF_OK;
}
wcfStatus wcfTextGet(wcfClient *wcli, const char *member, const char *field,
                     char *text, int size, int *recv_size) {
    *recv_size = 0;
    if (!field || size <= 0) {
        return WCF_INVALID_ARGUMENT;
    }
    text[0] = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto str = wcli_->member(member ? member : "").text(field).tryGet();
    if (str) {
        const std::string &str2 = *str;
        int copy_size = (size - 1) < static_cast<int>(str2.size())
                            ? (size - 1)
                            : str2.size();
        std::memcpy(text, str2.c_str(), copy_size * sizeof(char));
        text[copy_size] = 0;
        *recv_size = str2.size();
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}
}
