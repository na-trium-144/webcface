#pragma once
#include <algorithm>
#include <vector>
#include <webcface/c_wcf.h>
#include <webcface/client.h>

namespace webcface {
inline namespace c_wcf {
/*!
 * \brief wcfInitで作られたクライアントのリスト
 *
 * wcfInit時に追加、wcfClose時に削除する
 *
 */
inline std::vector<wcfClient> wcli_list;

/*!
 * \brief wcfFuncFetchCallで取得されたwcfFuncListenerHandlerのリスト
 *
 */
inline std::vector<void *> fetched_handlers;

/*!
 * \brief voidポインタからclientオブジェクトを復元
 *
 * \return
 * Clientのポインタ、またはwcliがwcfInitで生成されたポインタでなければnullptr
 *
 */
inline Client *getWcli(wcfClient wcli) {
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
inline const wcfFuncListenerHandler *getFuncListenerHandler(const void *handler) {
    if (std::find(fetched_handlers.begin(), fetched_handlers.end(), handler) ==
        fetched_handlers.end()) {
        return nullptr;
    }
    return static_cast<const wcfFuncListenerHandler *>(handler);
}

} // namespace c_wcf
} // namespace webcface

using namespace webcface;
