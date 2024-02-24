#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief クライアントを初期化する
 * \return Clientのポインタ
 *
 */
WEBCFACE_DLL wcfClient *wcfInit(const char *name, const char *host, int port);
/*!
 * \brief クライアントを初期化する
 *
 *  (サーバーのアドレスとポートはデフォルトになる)
 * \return Clientのポインタ
 *
 */
WEBCFACE_DLL wcfClient *wcfInitDefault(const char *name);

/*!
 * \brief 有効なClientのポインタであるかを返す
 * \return
 * wcliが正常にwcfInitされwcfCloseする前のポインタであれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsValid(wcfClient *wcli);
/*!
 * \brief Clientが接続されているかどうかを返す
 * \return wcliが正常にwcfInitされサーバーに接続できていれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsConnected(wcfClient *wcli);
/*!
 * \brief クライアントを閉じる
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfClose(wcfClient *wcli);
/*!
 * \brief 接続を開始する
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfStart(wcfClient *wcli);
/*!
 * \brief 送信用にセットしたデータをすべて送信キューに入れる。
 *
 * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
 *
 * サーバーに接続していない場合、wcfStart()を呼び出す。
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfSync(wcfClient *wcli);

#ifdef __cplusplus
}
#endif
