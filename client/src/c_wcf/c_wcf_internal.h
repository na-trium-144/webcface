#pragma once
#include <algorithm>
#include <vector>
#include "webcface/wcf.h"
#include "webcface/client.h"
#include "webcface/func.h"
#include "webcface/components.h"
#include "webcface/common/val_adaptor.h"

WEBCFACE_NS_BEGIN
inline namespace c_wcf {
template <typename CharT>
inline std::basic_string<CharT> strOrEmpty(const CharT *p) {
    if (p) {
        return p;
    } else {
        return std::basic_string<CharT>();
    }
}

/*!
 * \brief wcfFuncFetchCallで取得されたwcfFuncCallHandleのリスト
 *
 * wcfFuncCallHandleをnewして追加、returnかreject時にdeleteして削除
 *
 */
inline std::unordered_map<const wcfFuncCallHandle *, FuncCallHandle>
    fetched_handles;
inline std::unordered_map<const wcfFuncCallHandleW *, FuncCallHandle>
    fetched_handles_w;

/*!
 * \brief wcfFuncRun,
 * wcfFuncGetResultで取得されたwcfMultiValとValAdaptorのリスト
 *
 * wcfMultiValをnewし、このリスト内のvalAdapterへのポインタをもつ
 */
inline std::unordered_map<const wcfMultiVal *, ValAdaptor> func_val_list;
inline std::unordered_map<const wcfMultiValW *, ValAdaptor> func_val_list_w;

/*!
 * \brief wcfViewGetで取得されたwcfViewComponentとViewComponentBase
 */
inline std::unordered_map<const wcfViewComponent *,
                          const std::vector<ViewComponent>>
    view_list;
inline std::unordered_map<const wcfViewComponentW *,
                          const std::vector<ViewComponent>>
    view_list_w;

template <typename CharT>
struct CharType {};
template <>
struct CharType<char> {
    using CVal = wcfMultiVal;
    using CHandle = wcfFuncCallHandle;
    using CCallback = wcfFuncCallback;
    using CEventCallback1 = wcfEventCallback1;
    using CEventCallback2 = wcfEventCallback2;
    using CComponent = wcfViewComponent;
    static constexpr auto &fetchedHandles() { return fetched_handles; }
    static constexpr auto &funcValList() { return func_val_list; }
    static constexpr auto &viewList() { return view_list; }
};
template <>
struct CharType<wchar_t> {
    using CVal = wcfMultiValW;
    using CHandle = wcfFuncCallHandleW;
    using CCallback = wcfFuncCallbackW;
    using CEventCallback1 = wcfEventCallback1W;
    using CEventCallback2 = wcfEventCallback2W;
    using CComponent = wcfViewComponentW;
    static constexpr auto &fetchedHandles() { return fetched_handles_w; }
    static constexpr auto &funcValList() { return func_val_list_w; }
    static constexpr auto &viewList() { return view_list_w; }
};

/*!
 * \brief wcfInitで作られたクライアントのリスト
 *
 * wcfInit時にnewして追加、wcfClose時にdeleteして削除する
 *
 */
inline std::vector<wcfClient *> wcli_list;

/*!
 * \brief wcfFuncRunAsyncで取得されたwcfPromiseのリスト
 *
 * wcfFuncRunAsyncでnewし、GetResultやWaitResultでdelete
 *
 */
inline std::vector<Promise *> func_result_list;

/*!
 * \brief voidポインタからclientオブジェクトを復元
 *
 * \return
 * Clientのポインタ、またはwcliがwcfInitで生成されたポインタでなければnullptr
 *
 */
inline Client *getWcli(wcfClient *wcli) {
    if (std::find(wcli_list.begin(), wcli_list.end(), wcli) ==
        wcli_list.end()) {
        return nullptr;
    }
    return static_cast<Client *>(wcli);
}

inline Promise *getPromise(wcfPromise *res) {
    if (std::find(func_result_list.begin(), func_result_list.end(), res) ==
        func_result_list.end()) {
        return nullptr;
    }
    return static_cast<Promise *>(res);
}

template <typename MultiVal>
ValAdaptor fromCVal(const MultiVal &val) {
    if (val.as_str != nullptr) {
        return ValAdaptor(val.as_str);
    } else if (val.as_double != 0) {
        return ValAdaptor(val.as_double);
    } else {
        return ValAdaptor(val.as_int);
    }
}
template <typename CharT>
std::vector<ValAdaptor> argsFromCVal(const typename CharType<CharT>::CVal *args,
                                     int arg_size) {
    std::vector<ValAdaptor> args_v;
    args_v.reserve(arg_size);
    for (int i = 0; i < arg_size; i++) {
        args_v.emplace_back(fromCVal(args[i]));
    }
    return args_v;
}

template <typename CharT>
auto resultToCVal(const ValAdaptor &result_val) {
    auto result = new typename CharType<CharT>::CVal();
    CharType<CharT>::funcValList().emplace(result, result_val);
    const ValAdaptor &result_val_ref =
        CharType<CharT>::funcValList().at(result);
    result->as_int = result_val_ref;
    result->as_double = result_val_ref;
    result->as_str = result_val_ref;
    return result;
}

} // namespace c_wcf
WEBCFACE_NS_END

using namespace webcface;
