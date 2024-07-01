#pragma once
#include <webcface/common/def.h>
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
namespace Internal {
namespace WebSocket {
/*!
 * \brief WebSocketに接続する
 *
 * 成功すると
 * data->syncDataFirst()をsendし、
 * data->connectedをtrueにし、
 * data->current_curl_handleがnullptrでない値になる
 *
 */
WEBCFACE_DLL void init(const std::shared_ptr<Internal::ClientData> &data);
/*!
 * \brief 切断しhandleをfreeする
 *
 */
WEBCFACE_DLL void close(const std::shared_ptr<Internal::ClientData> &data);
/*!
 * \brief messageを1回受信しdata->onRecvを呼ぶ
 *
 */
WEBCFACE_DLL void recv(const std::shared_ptr<Internal::ClientData> &data,
                       const std::function<void(const std::string &)> &cb);
/*!
 * \brief メッセージを送信する
 *
 */
WEBCFACE_DLL void send(const std::shared_ptr<Internal::ClientData> &data,
                       const std::string &msg);

} // namespace WebSocket
} // namespace Internal
WEBCFACE_NS_END