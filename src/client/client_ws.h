#pragma once
#include <webcface/common/def.h>
#include "client_internal.h"

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
WEBCFACE_DLL void init(std::shared_ptr<Internal::ClientData> data);
/*!
 * \brief 切断しhandleをfreeする
 *
 */
WEBCFACE_DLL void close(std::shared_ptr<Internal::ClientData> data);
/*!
 * \brief messageを1回受信しdata->onRecvを呼ぶ
 *
 */
WEBCFACE_DLL void recv(std::shared_ptr<Internal::ClientData> data);
/*!
 * \brief メッセージを送信する
 *
 */
WEBCFACE_DLL void send(std::shared_ptr<Internal::ClientData> data,
                       const std::string &msg);

} // namespace WebSocket
} // namespace Internal
WEBCFACE_NS_END