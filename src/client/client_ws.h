#pragma once
#include <webcface/common/def.h>
#include "client_internal.h"

WEBCFACE_NS_BEGIN
namespace Internal {
namespace WebSocket {
/*!
 * \brief WebSocketに接続する
 *
 * use_cbがfalseの場合、接続を1回試行しreturn
 * 成功すると
 * data->current_curl_successをtrueにし、
 * initSuccess()を呼び、
 * data->current_curl_handle にhandleが入る
 *
 * use_cbがtrueの場合、接続後curl内部のループが回る
 * 切断されたときdeinit()を呼びreturnする
 *
 */
WEBCFACE_DLL void init(std::shared_ptr<Internal::ClientData> data, bool use_cb);
/*!
 * \brief 接続に成功したときの初期化処理
 *
 * init()の中で呼ばれ、
 * data->syncDataFirst()をsendし、
 * data->connectedをtrueにし、
 * data->current_curl_closedをfalseにする
 *
 */
WEBCFACE_DLL void initSuccess(Internal::ClientData *data);
/*!
 * \brief handleをfreeする
 *
 */
WEBCFACE_DLL void deinit(std::shared_ptr<Internal::ClientData> data);

/*!
 * \brief curlから呼ばれるコールバック
 *
 * 初回の呼び出しでinitSuccessを呼び、
 * それ以降はrecvFrameを呼ぶ
 *
 */
WEBCFACE_DLL size_t cb(char *buffer, size_t size, size_t nmemb, void *clientp);
/*!
 * \brief messageを1回受信しrecvFrameを呼ぶ
 *
 */
WEBCFACE_DLL void recv(std::shared_ptr<Internal::ClientData> data);
/*!
 * \brief メッセージを処理しdata->onRecvを呼ぶ
 *
 */
WEBCFACE_DLL void recvFrame(Internal::ClientData *data, char *buffer,
                            std::size_t rlen, const void *meta_v);

/*!
 * \brief メッセージを送信する
 *
 */
WEBCFACE_DLL void send(Internal::ClientData *data, const std::string &msg);
/*!
 * \brief curlws_closeを送る
 *
 */
WEBCFACE_DLL void close(Internal::ClientData *data);

} // namespace WebSocket
} // namespace Internal
WEBCFACE_NS_END