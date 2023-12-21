#include "c_wcli.h"

extern "C" {
wcfStatus wcfFuncRun(wcfClient wcli, const char *member, const char *field,
                     const wcfMultiVal *args, int arg_size,
                     wcfMultiVal **result) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    auto [status, res] = wcli_->member(member).func(field).runCVal(args_v);
    *result = res;
    return status;
}
wcfStatus wcfFuncRunS(wcfClient wcli, const char *member, const char *field,
                      const char **args, int arg_size, wcfMultiVal **result) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    auto [status, res] = wcli_->member(member).func(field).runCVal(args_v);
    *result = res;
    return status;
}
}
