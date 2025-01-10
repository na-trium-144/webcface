#pragma once
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
namespace internal {
namespace WebSocket {

/*!
 * \brief WebSocketに接続する
 *
 * 成功すると
 * data->connectedをtrueにし、
 * data->current_curl_handleがnullptrでない値になる
 *
 */
void init(const std::shared_ptr<internal::ClientData> &data);
/*!
 * \brief 切断しhandleをfreeする
 *
 */
void close(const std::shared_ptr<internal::ClientData> &data);
/*!
 * \brief messageを1回受信しdata->onRecvを呼ぶ
 *
 * なにか受信したらcallbackを呼んでtrue
 *
 */
bool recv(const std::shared_ptr<internal::ClientData> &data,
          const std::function<void(std::string &&)> &cb);
/*!
 * \brief メッセージを送信する
 *
 */
void send(const std::shared_ptr<internal::ClientData> &data,
          const std::string &msg);

} // namespace WebSocket
} // namespace internal
WEBCFACE_NS_END
