#pragma once
#include <algorithm>
#include <vector>
#include <webcface/c_wcf.h>
#include <webcface/client.h>

namespace WEBCFACE_NS {
inline namespace c_wcf {
/*!
 * \brief wcfInitで作られたクライアントのリスト
 *
 * wcfInit時に追加、wcfClose時に削除する
 *
 */
inline std::vector<wcfClient *> wcli_list;

/*!
 * \brief wcfFuncFetchCallで取得されたwcfFuncListenerHandlerのリスト
 *
 */
inline std::vector<void *> fetched_handles;

/*!
 * \brief wcfFuncRunAsyncで取得されたwcfAsyncFuncResultのリスト
 *
 */
inline std::vector<AsyncFuncResult *> func_result_list;

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
/*!
 * \brief voidポインタからFuncListenerHandlerオブジェクトを復元
 *
 */
inline const wcfFuncCallHandle *getFuncCallHandle(const void *handler) {
    if (std::find(fetched_handles.begin(), fetched_handles.end(), handler) ==
        fetched_handles.end()) {
        return nullptr;
    }
    return static_cast<const wcfFuncCallHandle *>(handler);
}

inline AsyncFuncResult *getAsyncFuncResult(wcfAsyncFuncResult *res) {
    if (std::find(func_result_list.begin(), func_result_list.end(), res) ==
        func_result_list.end()) {
        return nullptr;
    }
    return static_cast<AsyncFuncResult *>(res);
}

} // namespace c_wcf
} // namespace WEBCFACE_NS

using namespace WEBCFACE_NS;
