#include "c_wcf_internal.h"
#include <webcface/value.h>
#include <cstring>

extern "C" {
wcfStatus wcfValueSet(wcfClient *wcli, const char *field, double value) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    wcli_->value(field).set(value);
    return WCF_OK;
}
wcfStatus wcfValueSetVecD(wcfClient *wcli, const char *field,
                          const double *value, int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    wcli_->value(field).set(std::vector<double>(value, value + size));
    return WCF_OK;
}
wcfStatus wcfValueGetVecD(wcfClient *wcli, const char *member,
                          const char *field, double *values, int size,
                          int *recv_size) {
    *recv_size = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    auto vec = wcli_->member(member ? member : "").value(field).tryGetVec();
    if (vec) {
        int copy_size = size < static_cast<int>(vec->size())
                            ? size
                            : static_cast<int>(vec->size());
        std::memcpy(values, vec->data(), copy_size * sizeof(double));
        std::memset(values + copy_size, 0, (size - copy_size) * sizeof(double));
        *recv_size = static_cast<int>(vec->size());
        return WCF_OK;
    } else {
        std::memset(values, 0, size * sizeof(double));
        return WCF_NOT_FOUND;
    }
}
wcfStatus wcfValueGet(wcfClient *wcli, const char *member, const char *field,
                      double *value) {
    *value = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    auto val = wcli_->member(member ? member : "").value(field).tryGet();
    if (val) {
        *value = *val;
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}
}
