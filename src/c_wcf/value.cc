#include "c_wcf_internal.h"

extern "C" {
wcfStatus wcfValueSet(wcfClient wcli, const char *field, double value) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->value(field).set(value);
    return WCF_OK;
}
wcfStatus wcfValueSetVecD(wcfClient wcli, const char *field,
                          const double *value, int size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->value(field).set(std::vector<double>(value, value + size));
    return WCF_OK;
}
wcfStatus wcfValueGetVecD(wcfClient wcli, const char* member, const char* field,
    double* values, int size, int* recv_size) {
    *recv_size = 0;
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto vec = wcli_->member(member).value(field).tryGetVec();
    if (vec) {
        int copy_size = size < vec->size() ? size : vec->size();
        std::memcpy(values, vec->data(), copy_size * sizeof(double));
        std::memset(values + copy_size, 0,
                    (size - copy_size) * sizeof(double));
        *recv_size = vec->size();
        return WCF_OK;
    } else {
        return WCF_NOT_FOUND;
    }
}
}