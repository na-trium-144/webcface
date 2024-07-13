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
 * \brief サーバーへの接続を別スレッドで開始する。
 * \since ver1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfWaitConnection(), wcfAutoReconnect()
 */
WEBCFACE_DLL wcfStatus wcfStart(wcfClient *wcli);
/*!
 * \brief サーバーへの接続を別スレッドで開始し、成功するまで待機する。
 * \since ver2.0
 *
 * * wcfAutoReconnect が無効の場合は1回目の接続のみ待機し、
 * 失敗しても再接続せずreturnする。
 *
 * \param wcli
 * \param interval autoRecvが無効の場合、初期化が完了するまで一定間隔ごとに
 * wcfRecv() をこのスレッドで呼び出す。
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfStart(), wcfAutoReconnect()
 */
WEBCFACE_DLL wcfStatus wcfWaitConnection(wcfClient *wcli, int interval);
/*!
 * \brief 通信が切断されたときに自動で再試行するかどうかを設定する。
 * \since ver1.11.1
 * \param wcli
 * \param enabled 0以外にすると有効、0にすると無効になる。デフォルトは有効
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfStart()
 */
WEBCFACE_DLL wcfStatus wcfAutoReconnect(wcfClient *wcli, int enabled);
/*!
 * \brief サーバーからデータを受信する
 * \since ver2.0
 *
 * * データを受信した場合、各種コールバック(onEntry, onChange,
 * Funcなど)をこのスレッドで呼び出し、
 * それがすべて完了するまでこの関数はブロックされる。
 * * データを何も受信しなかった場合、サーバーに接続していない場合、
 * または接続試行中やデータ送信中など受信ができない場合は、
 * timeout経過後にreturnする。
 * timeout=0 または負の値なら即座にreturnする。
 * * timeoutが100μs以上の場合、データを何も受信できなければ100μsおきに再試行する。
 *
 * \param wcli
 * \param timeout (μs単位)
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfWaitRecv(), wcfAutoRecv()
 */
WEBCFACE_DLL wcfStatus wcfRecv(wcfClient *wcli, int timeout);
/*!
 * \brief サーバーからデータを受信する
 * \since ver2.0
 *
 * * wcfRecv()と同じだが、何か受信するまで無制限に待機する
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfRecv(), wcfAutoRecv()
 */
WEBCFACE_DLL wcfStatus wcfWaitRecv(wcfClient *wcli);
/*!
 * \brief 別スレッドでwcfRecv()を自動的に呼び出す間隔を設定する。
 * \since ver2.0
 *
 * * wcfStart() や wcfWaitConnection() より前に設定する必要がある。
 * * autoRecvが有効の場合、別スレッドで一定間隔ごとにrecv()が呼び出され、
 * 各種コールバック(onEntry, onChange,
 * Funcなど)も別のスレッドで呼ばれることになる
 * (そのためmutexなどを適切に設定すること)
 * * デフォルトでは無効なので、手動でrecv()を呼び出す必要がある
 *
 * \param wcli
 * \param interval (μs単位)
 * 1以上を指定するとその間隔で自動でrecv()が呼び出されるようになる。
 * 0または負の値を指定するとautoRecvは無効。
 * \sa recv(), recvUntil(), waitRecv()
 */
WEBCFACE_DLL wcfStatus wcfAutoRecv(wcfClient *wcli, int interval);
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