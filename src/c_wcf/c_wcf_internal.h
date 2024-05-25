#pragma once
#include <algorithm>
#include <vector>
#include <webcface/wcf.h>
#include <webcface/client.h>
#include <webcface/func.h>
#include <webcface/canvas_data.h>
#include <webcface/common/val.h>

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
inline std::unordered_map<const wcfMultiVal *, Common::ValAdaptor>
    func_val_list;
inline std::unordered_map<const wcfMultiValW *, Common::ValAdaptor>
    func_val_list_w;

template <typename CharT>
struct CharType {};
template <>
struct CharType<char> {
    using CVal = wcfMultiVal;
    using CHandle = wcfFuncCallHandle;
    using CCallback = wcfFuncCallback;
    static constexpr auto &fetchedHandles() { return fetched_handles; }
    static constexpr auto &funcValList() { return func_val_list; }
};
template <>
struct CharType<wchar_t> {
    using CVal = wcfMultiValW;
    using CHandle = wcfFuncCallHandleW;
    using CCallback = wcfFuncCallbackW;
    static constexpr auto &fetchedHandles() { return fetched_handles_w; }
    static constexpr auto &funcValList() { return func_val_list_w; }
};

/*!
 * \brief wcfInitで作られたクライアントのリスト
 *
 * wcfInit時にnewして追加、wcfClose時にdeleteして削除する
 *
 */
inline std::vector<wcfClient *> wcli_list;

/*!
 * \brief wcfFuncRunAsyncで取得されたwcfAsyncFuncResultのリスト
 *
 * wcfFuncRunAsyncでnewし、GetResultやWaitResultでdelete
 *
 */
inline std::vector<AsyncFuncResult *> func_result_list;

/*!
 * \brief wcfViewGetで取得されたwcfViewComponentとViewComponentBase
 */
inline std::unordered_map<const wcfViewComponent *,
                          const std::vector<ViewComponent>>
    view_list;

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

inline AsyncFuncResult *getAsyncFuncResult(wcfAsyncFuncResult *res) {
    if (std::find(func_result_list.begin(), func_result_list.end(), res) ==
        func_result_list.end()) {
        return nullptr;
    }
    return static_cast<AsyncFuncResult *>(res);
}

} // namespace c_wcf
WEBCFACE_NS_END

using namespace webcface;
