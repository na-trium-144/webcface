#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief クライアントを初期化する
 * \since 1.5
 * \return Clientのポインタ
 *
 */
WEBCFACE_DLL wcfClient *wcfInit(const char *name, const char *host, int port);
/*!
 * \brief クライアントを初期化する
 * \since 1.5
 *
 *  (サーバーのアドレスとポートはデフォルトになる)
 * \return Clientのポインタ
 *
 */
WEBCFACE_DLL wcfClient *wcfInitDefault(const char *name);

/*!
 * \brief 有効なClientのポインタであるかを返す
 * \since 1.5
 * \return
 * wcliが正常にwcfInitされwcfCloseする前のポインタであれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsValid(wcfClient *wcli);
/*!
 * \brief Clientが接続されているかどうかを返す
 * \since 1.5
 * \return wcliが正常にwcfInitされサーバーに接続できていれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsConnected(wcfClient *wcli);
/*!
 * \brief クライアントを閉じる
 * \since 1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfClose(wcfClient *wcli);
/*!
 * \brief 接続を開始する
 * \since 1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfStart(wcfClient *wcli);
/*!
 * \brief 送信用にセットしたデータをすべて送信キューに入れる。
 * \since 1.5
 *
 * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
 *
 * サーバーに接続していない場合、wcfStart()を呼び出す。
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfSync(wcfClient *wcli);

/*!
 * \brief wcfの関数から取得したポインタのデータを破棄
 * \since 1.7
 *
 * \param ptr データを格納したポインタ
 * \return ptrが wcfFuncRun, wcfFuncGetResult, wcfFuncWaitResult, wcfViewGet
 * で取得したものでない場合WCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus wcfDestroy(const void *ptr);

#ifdef __cplusplus
}
#endif
