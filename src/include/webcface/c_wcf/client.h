#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief クライアントを初期化する
 * \since ver1.5
 * \param name 名前 (ver1.7〜:NULLも可)
 * \param host サーバーのアドレス
 * \param port サーバーのポート
 * \return Clientのポインタ
 *
 */
WEBCFACE_DLL wcfClient *wcfInit(const char *name, const char *host, int port);
/*!
 * \brief クライアントを初期化する (wstring)
 * \since ver2.0
 * \sa wcfInit
 */
WEBCFACE_DLL wcfClient *wcfInitW(const wchar_t *name, const wchar_t *host,
                                 int port);
/*!
 * \brief クライアントを初期化する (アドレスとポートはデフォルト)
 * \since ver1.5
 * \param name 名前 (ver1.7〜:NULLも可)
 * \return Clientのポインタ
 */
WEBCFACE_DLL wcfClient *wcfInitDefault(const char *name);
/*!
 * \brief クライアントを初期化する (アドレスとポートはデフォルト, wstring)
 * \since ver2.0
 * \sa wcfInitDefault
 */
WEBCFACE_DLL wcfClient *wcfInitDefaultW(const wchar_t *name);

/*!
 * \brief 有効なClientのポインタであるかを返す
 * \since ver1.5
 * \return
 * wcliが正常にwcfInitされwcfCloseする前のポインタであれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsValid(wcfClient *wcli);
/*!
 * \brief Clientが接続されているかどうかを返す
 * \since ver1.5
 * \return wcliが正常にwcfInitされサーバーに接続できていれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsConnected(wcfClient *wcli);
/*!
 * \brief クライアントを閉じる
 * \since ver1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfClose(wcfClient *wcli);
/*!
 * \brief 接続を開始する
 * \since ver1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfStart(wcfClient *wcli);
/*!
 * \brief 送信用にセットしたデータをすべて送信キューに入れる。
 * \since ver1.5
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
 * \since ver1.7
 * \param ptr データを格納したポインタ
 * \return ptrが wcfFuncRun, wcfFuncGetResult, wcfFuncWaitResult, wcfViewGet
 * で取得したものでない場合WCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus wcfDestroy(const void *ptr);

#ifdef __cplusplus
}
#endif
