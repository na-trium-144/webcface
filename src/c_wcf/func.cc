#include "c_wcf_internal.h"

template <typename ArgPtr>
static inline wcfStatus wcfFuncRunImpl(wcfClient wcli, const char *member,
                                       const char *field, ArgPtr args,
                                       int arg_size, wcfMultiVal **result) {
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

extern "C" {
wcfStatus wcfFuncRun(wcfClient wcli, const char *member, const char *field,
                     const wcfMultiVal *args, int arg_size,
                     wcfMultiVal **result) {
    return wcfFuncRunImpl(wcli, member, field, args, arg_size, result);
}
wcfStatus wcfFuncRunS(wcfClient wcli, const char *member, const char *field,
                      const char **args, int arg_size, wcfMultiVal **result) {
    return wcfFuncRunImpl(wcli, member, field, args, arg_size, result);
}
wcfStatus wcfFuncRunD(wcfClient wcli, const char *member, const char *field,
                      const double *args, int arg_size, wcfMultiVal **result) {
    return wcfFuncRunImpl(wcli, member, field, args, arg_size, result);
}
wcfStatus wcfFuncRunI(wcfClient wcli, const char *member, const char *field,
                      const int *args, int arg_size, wcfMultiVal **result) {
    return wcfFuncRunImpl(wcli, member, field, args, arg_size, result);
}

wcfStatus wcfFuncListen(wcfClient wcli, const char *field,
                        const wcfValType *arg_types, int arg_size,
                        wcfValType return_type) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
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
                           wcfFuncListenerHandler **handler) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto h = wcli_->funcListener(field).fetchCall();
    if (h) {
        auto hp = new FuncListenerHandler(*h);
        auto whp = new wcfFuncListenerHandler();
        whp->args = hp->toCArgs().data();
        whp->arg_size = hp->args().size();
        whp->handler = static_cast<void *>(hp);
        *handler = whp;
        fetched_handlers.push_back(whp);
        return WCF_OK;
    } else {
        return WCF_NOT_CALLED;
    }
}
wcfStatus wcfFuncRespond(wcfClient wcli, const wcfFuncListenerHandler *handler,
                         const wcfMultiVal *value) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto wh_ = getFuncListenerHandler(handler);
    if (!wh_) {
        return WCF_BAD_HANDLER;
    }
    auto h_ = static_cast<FuncListenerHandler *>(wh_->handler);
    h_->respond(value);
    fetched_handlers.erase(
        std::find(fetched_handlers.begin(), fetched_handlers.end(), handler));
    delete h_;
    delete wh_;
    return WCF_OK;
}
wcfStatus wcfFuncReject(wcfClient wcli, const wcfFuncListenerHandler *handler,
                        const char *message) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    auto wh_ = getFuncListenerHandler(handler);
    if (!wh_) {
        return WCF_BAD_HANDLER;
    }
    auto h_ = static_cast<FuncListenerHandler *>(wh_->handler);
    h_->reject(message);
    fetched_handlers.erase(
        std::find(fetched_handlers.begin(), fetched_handlers.end(), handler));
    delete h_;
    delete wh_;
    return WCF_OK;
}
}
