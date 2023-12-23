#include "c_wcf_internal.h"

extern "C" {
wcfStatus wcfFuncRun(wcfClient wcli, const char *member, const char *field,
                     const wcfMultiVal *args, int arg_size,
                     wcfMultiVal **result) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    auto [status, res] = wcli_->member(member).func(field).runCVal(args_v);
    *result = res;
    return status;
}

wcfStatus wcfFuncListen(wcfClient wcli, const char *field, const int *arg_types,
                        int arg_size, int return_type) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    std::vector<Arg> args(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args[i].type(static_cast<ValType>(arg_types[i]));
    }
    wcli_->funcListener(field)
        .setArgs(args)
        .setReturnType(static_cast<ValType>(return_type))
        .listen();
    return WCF_OK;
}
wcfStatus wcfFuncFetchCall(wcfClient wcli, const char *field,
                           wcfFuncCallHandle **handle) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto h = wcli_->funcListener(field).fetchCall();
    if (h) {
        auto hp = new FuncCallHandle(*h);
        auto whp = new wcfFuncCallHandle{
            .args = hp->toCArgs().data(),
            .arg_size = static_cast<int>(hp->args().size()),
            .handle = static_cast<void *>(hp),
        };
        *handle = whp;
        fetched_handles.push_back(whp);
        return WCF_OK;
    } else {
        return WCF_NOT_CALLED;
    }
}

wcfStatus wcfFuncRespond(const wcfFuncCallHandle *handle,
                         const wcfMultiVal *value) {
    auto wh_ = getFuncCallHandle(handle);
    if (!wh_) {
        return WCF_BAD_HANDLE;
    }
    auto h_ = static_cast<FuncCallHandle *>(wh_->handle);
    h_->respond(value);
    fetched_handles.erase(
        std::find(fetched_handles.begin(), fetched_handles.end(), handle));
    delete h_;
    delete wh_;
    return WCF_OK;
}
wcfStatus wcfFuncReject(const wcfFuncCallHandle *handle, const char *message) {
    auto wh_ = getFuncCallHandle(handle);
    if (!wh_) {
        return WCF_BAD_HANDLE;
    }
    auto h_ = static_cast<FuncCallHandle *>(wh_->handle);
    h_->reject(message);
    fetched_handles.erase(
        std::find(fetched_handles.begin(), fetched_handles.end(), handle));
    delete h_;
    delete wh_;
    return WCF_OK;
}
}
