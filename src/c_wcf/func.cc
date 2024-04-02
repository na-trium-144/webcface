#include "c_wcf_internal.h"
#include <webcface/func.h>

static std::pair<wcfStatus, wcfMultiVal *>
resultToCVal(AsyncFuncResult async_res) {
    ValAdaptor result_val;
    wcfStatus status;
    try {
        result_val = async_res.result.get();
        status = WCF_OK;
    } catch (const FuncNotFound &e) {
        result_val = e.what();
        status = WCF_NOT_FOUND;
    } catch (const std::exception &e) {
        result_val = e.what();
        status = WCF_EXCEPTION;
    } catch (...) {
        result_val = "unknown exception";
        status = WCF_EXCEPTION;
    }
    wcfMultiVal *result = new wcfMultiVal();
    func_val_list.emplace(result, result_val);
    const ValAdaptor &result_val_ref = func_val_list.at(result);
    result->as_int = result_val_ref;
    result->as_double = result_val_ref;
    result->as_str = static_cast<const std::string &>(result_val_ref).c_str();
    return std::make_pair(status, result);
}

static wcfFuncCallHandle *createHandle(const FuncCallHandle &h) {
    auto whp = new wcfFuncCallHandle{};
    fetched_handles.emplace(whp, h);
    auto &h_ref = fetched_handles.at(whp);
    whp->args = h_ref.cArgs();
    whp->arg_size = static_cast<int>(h_ref.args().size());
    return whp;
}

extern "C" {
wcfStatus wcfFuncRun(wcfClient *wcli, const char *member, const char *field,
                     const wcfMultiVal *args, int arg_size,
                     wcfMultiVal **result) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    auto [status, result_p] = resultToCVal(
        wcli_->member(member ? member : "").func(field).runAsync(args_v));
    *result = result_p;
    return status;
}

wcfStatus wcfFuncRunAsync(wcfClient *wcli, const char *member,
                          const char *field, const wcfMultiVal *args,
                          int arg_size, wcfAsyncFuncResult **async_res) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field || arg_size < 0) {
        return WCF_INVALID_ARGUMENT;
    }
    std::vector<ValAdaptor> args_v(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v[i] = args[i];
    }
    AsyncFuncResult *a_res = new AsyncFuncResult(
        wcli_->member(member ? member : "").func(field).runAsync(args_v));
    func_result_list.push_back(a_res);
    *async_res = a_res;
    return WCF_OK;
}

wcfStatus wcfFuncGetResult(wcfAsyncFuncResult *async_res,
                           wcfMultiVal **result) {
    auto res = getAsyncFuncResult(async_res);
    if (!res) {
        return WCF_BAD_HANDLE;
    }
    if (res->result.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
        return WCF_NOT_RETURNED;
    }
    auto [status, result_p] = resultToCVal(*res);
    *result = result_p;
    func_result_list.erase(
        std::find(func_result_list.begin(), func_result_list.end(), res));
    delete res;
    return status;
}
wcfStatus wcfFuncWaitResult(wcfAsyncFuncResult *async_res,
                            wcfMultiVal **result) {
    auto res = getAsyncFuncResult(async_res);
    if (!res) {
        return WCF_BAD_HANDLE;
    }
    auto [status, result_p] = resultToCVal(*res);
    *result = result_p;
    func_result_list.erase(
        std::find(func_result_list.begin(), func_result_list.end(), res));
    delete res;
    return status;
}

wcfStatus wcfFuncSet(wcfClient *wcli, const char *field, const int *arg_types,
                     int arg_size, int return_type, wcfFuncCallback callback,
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
    wcli_->func(field).set(args, static_cast<ValType>(return_type),
                           [callback, user_data](FuncCallHandle handle) {
                               wcfFuncCallHandle *whp = createHandle(handle);
                               callback(whp, user_data);
                               wcfFuncRespond(whp, nullptr);
                           });
    return WCF_OK;
}
wcfStatus wcfFuncListen(wcfClient *wcli, const char *field,
                        const int *arg_types, int arg_size, int return_type) {
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
wcfStatus wcfFuncFetchCall(wcfClient *wcli, const char *field,
                           wcfFuncCallHandle **handle) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    if (!field) {
        return WCF_INVALID_ARGUMENT;
    }
    auto h = wcli_->funcListener(field).fetchCall();
    if (h) {
        *handle = createHandle(*h);
        return WCF_OK;
    } else {
        return WCF_NOT_CALLED;
    }
}

wcfStatus wcfFuncRespond(const wcfFuncCallHandle *handle,
                         const wcfMultiVal *value) {
    auto it = fetched_handles.find(handle);
    if (it == fetched_handles.end()) {
        return WCF_BAD_HANDLE;
    }
    if (value) {
        it->second.respond(*value);
    } else {
        it->second.respond();
    }
    fetched_handles.erase(it);
    delete handle;
    return WCF_OK;
}
wcfStatus wcfFuncReject(const wcfFuncCallHandle *handle, const char *message) {
    auto it = fetched_handles.find(handle);
    if (it == fetched_handles.end()) {
        return WCF_BAD_HANDLE;
    }
    it->second.reject(message ? message : "");
    fetched_handles.erase(it);
    delete handle;
    return WCF_OK;
}
}
