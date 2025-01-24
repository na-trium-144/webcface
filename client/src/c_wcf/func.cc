#include "c_wcf_internal.h"
#include "webcface/func.h"

/// \private
template <typename CharT>
static auto createHandle(const FuncCallHandle &h) {
    auto whp = new typename CharType<CharT>::CHandle();
    CharType<CharT>::fetchedHandles().emplace(whp, h);
    auto &h_ref = CharType<CharT>::fetchedHandles().at(whp);
    if constexpr (std::is_same_v<CharT, char>) {
        whp->args = h_ref.cArgs();
    } else {
        whp->args = h_ref.cWArgs();
    }
    whp->arg_size = static_cast<int>(h_ref.args().size());
    return whp;
}

/// \private
template <typename CharT>
static wcfStatus
wcfFuncRunT(wcfClient *wcli, const CharT *member, const CharT *field,
            const typename CharType<CharT>::CVal *args, int arg_size,
            typename CharType<CharT>::CVal **result) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    Promise p = wcli_->member(strOrEmpty(member))
                    .func(field)
                    .runAsync(argsFromCVal<CharT>(args, arg_size));
    p.waitFinish();
    if (!p.found()) {
        *result = resultToCVal<CharT>(ValAdaptor(p.rejection()));
        return WCF_NOT_FOUND;
    } else if (p.isError()) {
        *result = resultToCVal<CharT>(ValAdaptor(p.rejection()));
        return WCF_EXCEPTION;
    } else {
        *result = resultToCVal<CharT>(p.response());
        return WCF_OK;
    }
}
/// \private
template <typename CharT>
static wcfStatus
wcfFuncRunAsyncT(wcfClient *wcli, const CharT *member, const CharT *field,
                 const typename CharType<CharT>::CVal *args, int arg_size,
                 wcfPromise **async_res) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    Promise *a_res =
        new Promise(wcli_->member(strOrEmpty(member))
                                .func(field)
                                .runAsync(argsFromCVal<CharT>(args, arg_size)));
    func_result_list.push_back(a_res);
    *async_res = a_res;
    return WCF_OK;
}
/// \private
template <typename CharT>
static wcfStatus wcfFuncGetResultT(wcfPromise *async_res,
                                   typename CharType<CharT>::CVal **result,
                                   bool non_block) {
    auto res = getPromise(async_res);
    if (!res) {
        return WCF_BAD_HANDLE;
    }
    if (non_block && !res->finished()) {
        return WCF_NOT_RETURNED;
    }
    wcfStatus status;
    res->waitFinish();
    if (!res->found()) {
        *result = resultToCVal<CharT>(ValAdaptor(res->rejection()));
        status = WCF_NOT_FOUND;
    } else if (res->isError()) {
        *result = resultToCVal<CharT>(ValAdaptor(res->rejection()));
        status = WCF_EXCEPTION;
    } else {
        *result = resultToCVal<CharT>(res->response());
        status = WCF_OK;
    }
    func_result_list.erase(
        std::find(func_result_list.begin(), func_result_list.end(), res));
    delete res;
    return status;
}

template <typename CharT>
wcfStatus wcfFuncRespondT(const typename CharType<CharT>::CHandle *handle,
                          const typename CharType<CharT>::CVal *value) {
    auto it = CharType<CharT>::fetchedHandles().find(handle);
    if (it == CharType<CharT>::fetchedHandles().end()) {
        return WCF_BAD_HANDLE;
    }
    if (value) {
        it->second.respond(fromCVal(*value));
    } else {
        it->second.respond();
    }
    CharType<CharT>::fetchedHandles().erase(it);
    delete handle;
    return WCF_OK;
}
/// \private
template <typename CharT>
static wcfStatus wcfFuncRejectT(const typename CharType<CharT>::CHandle *handle,
                                const CharT *message) {
    auto it = CharType<CharT>::fetchedHandles().find(handle);
    if (it == CharType<CharT>::fetchedHandles().end()) {
        return WCF_BAD_HANDLE;
    }
    it->second.reject(strOrEmpty(message));
    CharType<CharT>::fetchedHandles().erase(it);
    delete handle;
    return WCF_OK;
}

/// \private
template <typename CharT>
static wcfStatus
wcfFuncSetT(wcfClient *wcli, const CharT *field, const wcfValType *arg_types,
            int arg_size, wcfValType return_type,
            typename CharType<CharT>::CCallback callback, void *user_data) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    std::vector<Arg> args(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args[i].type(static_cast<ValType>(arg_types[i]));
    }
    wcli_->func(field).set(args, static_cast<ValType>(return_type),
                           [callback, user_data](const FuncCallHandle &handle) {
                               auto whp = createHandle<CharT>(handle);
                               callback(whp, user_data);
                           });
    return WCF_OK;
}
/// \private
template <typename CharT>
static wcfStatus wcfFuncSetAsyncT(wcfClient *wcli, const CharT *field,
                                  const wcfValType *arg_types, int arg_size,
                                  wcfValType return_type,
                                  typename CharType<CharT>::CCallback callback,
                                  void *user_data) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    std::vector<Arg> args(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args[i].type(static_cast<ValType>(arg_types[i]));
    }
    wcli_->func(field).setAsync(
        args, static_cast<ValType>(return_type),
        [callback, user_data](const FuncCallHandle &handle) {
            auto whp = createHandle<CharT>(handle);
            callback(whp, user_data);
        });
    return WCF_OK;
}
/// \private
template <typename CharT>
static wcfStatus wcfFuncListenT(wcfClient *wcli, const CharT *field,
                                const wcfValType *arg_types, int arg_size,
                                wcfValType return_type) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
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
/// \private
template <typename CharT>
static wcfStatus wcfFuncFetchCallT(wcfClient *wcli, const CharT *field,
                                   typename CharType<CharT>::CHandle **handle) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    auto h = wcli_->funcListener(field).fetchCall();
    if (h) {
        *handle = createHandle<CharT>(*h);
        return WCF_OK;
    } else {
        return WCF_NOT_CALLED;
    }
}

extern "C" {
wcfStatus wcfFuncRun(wcfClient *wcli, const char *member, const char *field,
                     const wcfMultiVal *args, int arg_size,
                     wcfMultiVal **result) {
    return wcfFuncRunT(wcli, member, field, args, arg_size, result);
}
wcfStatus wcfFuncRunW(wcfClient *wcli, const wchar_t *member,
                      const wchar_t *field, const wcfMultiValW *args,
                      int arg_size, wcfMultiValW **result) {
    return wcfFuncRunT(wcli, member, field, args, arg_size, result);
}
wcfStatus wcfFuncRunAsync(wcfClient *wcli, const char *member,
                          const char *field, const wcfMultiVal *args,
                          int arg_size, wcfPromise **async_res) {
    return wcfFuncRunAsyncT(wcli, member, field, args, arg_size, async_res);
}
wcfStatus wcfFuncRunAsyncW(wcfClient *wcli, const wchar_t *member,
                           const wchar_t *field, const wcfMultiValW *args,
                           int arg_size, wcfPromise **async_res) {
    return wcfFuncRunAsyncT(wcli, member, field, args, arg_size, async_res);
}

wcfStatus wcfFuncGetResult(wcfPromise *async_res,
                           wcfMultiVal **result) {
    return wcfFuncGetResultT<char>(async_res, result, true);
}
wcfStatus wcfFuncWaitResult(wcfPromise *async_res,
                            wcfMultiVal **result) {
    return wcfFuncGetResultT<char>(async_res, result, false);
}

wcfStatus wcfFuncSet(wcfClient *wcli, const char *field,
                     const wcfValType *arg_types, int arg_size,
                     wcfValType return_type, wcfFuncCallback callback,
                     void *user_data) {
    return wcfFuncSetT(wcli, field, arg_types, arg_size, return_type, callback,
                       user_data);
}
wcfStatus wcfFuncSetW(wcfClient *wcli, const wchar_t *field,
                      const wcfValType *arg_types, int arg_size,
                      wcfValType return_type, wcfFuncCallbackW callback,
                      void *user_data) {
    return wcfFuncSetT(wcli, field, arg_types, arg_size, return_type, callback,
                       user_data);
}
wcfStatus wcfFuncSetAsync(wcfClient *wcli, const char *field,
                          const wcfValType *arg_types, int arg_size,
                          wcfValType return_type, wcfFuncCallback callback,
                          void *user_data) {
    return wcfFuncSetAsyncT(wcli, field, arg_types, arg_size, return_type,
                            callback, user_data);
}
wcfStatus wcfFuncSetAsyncW(wcfClient *wcli, const wchar_t *field,
                           const wcfValType *arg_types, int arg_size,
                           wcfValType return_type, wcfFuncCallbackW callback,
                           void *user_data) {
    return wcfFuncSetAsyncT(wcli, field, arg_types, arg_size, return_type,
                            callback, user_data);
}
wcfStatus wcfFuncListen(wcfClient *wcli, const char *field,
                        const wcfValType *arg_types, int arg_size,
                        wcfValType return_type) {
    return wcfFuncListenT(wcli, field, arg_types, arg_size, return_type);
}
wcfStatus wcfFuncListenW(wcfClient *wcli, const wchar_t *field,
                         const wcfValType *arg_types, int arg_size,
                         wcfValType return_type) {
    return wcfFuncListenT(wcli, field, arg_types, arg_size, return_type);
}
wcfStatus wcfFuncFetchCall(wcfClient *wcli, const char *field,
                           wcfFuncCallHandle **handle) {
    return wcfFuncFetchCallT(wcli, field, handle);
}
wcfStatus wcfFuncFetchCallW(wcfClient *wcli, const wchar_t *field,
                            wcfFuncCallHandleW **handle) {
    return wcfFuncFetchCallT(wcli, field, handle);
}
wcfStatus wcfFuncRespond(const wcfFuncCallHandle *handle,
                         const wcfMultiVal *value) {
    return wcfFuncRespondT<char>(handle, value);
}
wcfStatus wcfFuncRespondW(const wcfFuncCallHandleW *handle,
                          const wcfMultiValW *value) {
    return wcfFuncRespondT<wchar_t>(handle, value);
}
wcfStatus wcfFuncReject(const wcfFuncCallHandle *handle, const char *message) {
    return wcfFuncRejectT(handle, message);
}
wcfStatus wcfFuncRejectW(const wcfFuncCallHandleW *handle,
                         const wchar_t *message) {
    return wcfFuncRejectT(handle, message);
}

// todo: args, returnType の取得と設定
}
