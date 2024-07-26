#include "c_wcf_internal.h"
#include "webcface/value.h"
#include <cstring>

/// \private
template <typename CharT>
static wcfStatus wcfValueSetT(wcfClient *wcli, const CharT *field,
                              double value) {
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
/// \private
template <typename CharT>
static wcfStatus wcfValueSetVecDT(wcfClient *wcli, const CharT *field,
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

/// \private
template <typename CharT>
static wcfStatus wcfValueGetVecDT(wcfClient *wcli, const CharT *member,
                                  const CharT *field, double *values, int size,
                                  int *recv_size) {
    *recv_size = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || size < 0) {
        return WCF_INVALID_ARGUMENT;
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
        return WCF_OK;
    } else {
        if (size > 0) {
            std::memset(values, 0, size * sizeof(double));
        }
        return WCF_NO_DATA;
    }
}
/// \private
template <typename CharT>
static wcfStatus wcfValueGetT(wcfClient *wcli, const CharT *member,
                              const CharT *field, double *value) {
    *value = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    auto val = wcli_->member(strOrEmpty(member)).value(field).tryGet();
    if (val) {
        *value = *val;
        return WCF_OK;
    } else {
        return WCF_NO_DATA;
    }
}
/// \private
template <typename CharT>
static wcfStatus
wcfValueChangeEventT(wcfClient *wcli, const CharT *member, const CharT *field,
                     typename CharType<CharT>::CEventCallback2 callback,
                     void *user_data) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->member(strOrEmpty(member))
        .value(field)
        .onChange([callback, user_data](const Value &m) {
            if constexpr (std::is_same_v<CharT, char>) {
                callback(m.member().name().c_str(), m.name().c_str(),
                         user_data);
            } else {
                callback(m.member().nameW().c_str(), m.nameW().c_str(),
                         user_data);
            }
        });
    return WCF_OK;
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
wcfStatus wcfValueChangeEvent(wcfClient *wcli, const char *member,
                              const char *field, wcfEventCallback2 callback,
                              void *user_data) {
    return wcfValueChangeEventT(wcli, member, field, callback, user_data);
}
wcfStatus wcfValueChangeEventW(wcfClient *wcli, const wchar_t *member,
                               const wchar_t *field,
                               wcfEventCallback2W callback, void *user_data) {
    return wcfValueChangeEventT(wcli, member, field, callback, user_data);
}
}
