#include "c_wcli.h"

extern "C" {
const wcfMultiVal *wcfFuncRun(wcfClient wcli, const char *member,
                              const char *field, wcfMultiVal *args,
                              int arg_size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return nullptr;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    return &wcli_->member(member).func(field).runCVal(args_v);
}
const char *wcfFuncRunS(wcfClient wcli, const char *member, const char *field,
                        const char **args, int arg_size) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return nullptr;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    return wcli_->member(member).func(field).runCVal(args_v).as_str;
}
}
