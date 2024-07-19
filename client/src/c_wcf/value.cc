#include "c_wcf_internal.h"
#include "webcface/value.h"
#include <cstring>

template <typename CharT>
static wcfStatus wcfValueSetT(wcfClient *wcli, const CharT *field,
                              double value) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    if (!field) {
        return wcfInvalidArgument;
    }
    wcli_->value(field).set(value);
    return wcfOk;
}
template <typename CharT>
static wcfStatus wcfValueSetVecDT(wcfClient *wcli, const CharT *field,
                                  const double *value, int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    if (!field) {
        return wcfInvalidArgument;
    }
    wcli_->value(field).set(std::vector<double>(value, value + size));
    return wcfOk;
}

template <typename CharT>
static wcfStatus wcfValueGetVecDT(wcfClient *wcli, const CharT *member,
                                  const CharT *field, double *values, int size,
                                  int *recv_size) {
    *recv_size = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    if (!field || size < 0) {
        return wcfInvalidArgument;
    }
    auto vec = wcli_->member(strOrEmpty(member)).value(field).tryGetVec();
    if (vec) {
        if (size > 0) {
            int copy_size = size < static_cast<int>(vec->size())
                                ? size
                                : static_cast<int>(vec->size());
            std::memcpy(values, vec->data(), copy_size * sizeof(double));
            std::memset(values + copy_size, 0,
                        (size - copy_size) * sizeof(double));
        }
        *recv_size = static_cast<int>(vec->size());
        return wcfOk;
    } else {
        if (size > 0) {
            std::memset(values, 0, size * sizeof(double));
        }
        return wcfNotFound;
    }
}
template <typename CharT>
static wcfStatus wcfValueGetT(wcfClient *wcli, const CharT *member,
                              const CharT *field, double *value) {
    *value = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    if (!field) {
        return wcfInvalidArgument;
    }
    auto val = wcli_->member(strOrEmpty(member)).value(field).tryGet();
    if (val) {
        *value = *val;
        return wcfOk;
    } else {
        return wcfNotFound;
    }
}

extern "C" {
wcfStatus wcfValueSet(wcfClient *wcli, const char *field, double value) {
    return wcfValueSetT(wcli, field, value);
}
wcfStatus wcfValueSetW(wcfClient *wcli, const wchar_t *field, double value) {
    return wcfValueSetT(wcli, field, value);
}
wcfStatus wcfValueSetVecD(wcfClient *wcli, const char *field,
                          const double *value, int size) {
    return wcfValueSetVecDT(wcli, field, value, size);
}
wcfStatus wcfValueSetVecDW(wcfClient *wcli, const wchar_t *field,
                           const double *value, int size) {
    return wcfValueSetVecDT(wcli, field, value, size);
}
wcfStatus wcfValueGetVecD(wcfClient *wcli, const char *member,
                          const char *field, double *values, int size,
                          int *recv_size) {
    return wcfValueGetVecDT(wcli, member, field, values, size, recv_size);
}
wcfStatus wcfValueGetVecDW(wcfClient *wcli, const wchar_t *member,
                           const wchar_t *field, double *values, int size,
                           int *recv_size) {
    return wcfValueGetVecDT(wcli, member, field, values, size, recv_size);
}
wcfStatus wcfValueGet(wcfClient *wcli, const char *member, const char *field,
                      double *value) {
    return wcfValueGetT(wcli, member, field, value);
}
wcfStatus wcfValueGetW(wcfClient *wcli, const wchar_t *member,
                       const wchar_t *field, double *value) {
    return wcfValueGetT(wcli, member, field, value);
}
}
