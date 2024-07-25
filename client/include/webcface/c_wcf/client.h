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
WEBCFACE_DLL wcfClient *WEBCFACE_CALL wcfInit(const char *name,
                                              const char *host, int port);
/*!
 * \brief クライアントを初期化する (wstring)
 * \since ver2.0
 * \sa wcfInit
 */
WEBCFACE_DLL wcfClient *WEBCFACE_CALL wcfInitW(const wchar_t *name,
                                               const wchar_t *host, int port);
/*!
 * \brief クライアントを初期化する (アドレスとポートはデフォルト)
 * \since ver1.5
 * \param name 名前 (ver1.7〜:NULLも可)
 * \return Clientのポインタ
 */
WEBCFACE_DLL wcfClient *WEBCFACE_CALL wcfInitDefault(const char *name);
/*!
 * \brief クライアントを初期化する (アドレスとポートはデフォルト, wstring)
 * \since ver2.0
 * \sa wcfInitDefault
 */
WEBCFACE_DLL wcfClient *WEBCFACE_CALL wcfInitDefaultW(const wchar_t *name);

/*!
 * \brief 有効なClientのポインタであるかを返す
 * \since ver1.5
 * \return
 * wcliが正常にwcfInitされwcfCloseする前のポインタであれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int WEBCFACE_CALL wcfIsValid(wcfClient *wcli);
/*!
 * \brief Clientが接続されているかどうかを返す
 * \since ver1.5
 * \return wcliが正常にwcfInitされサーバーに接続できていれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int WEBCFACE_CALL wcfIsConnected(wcfClient *wcli);
/*!
 * \brief クライアントを閉じる
 * \since ver1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfClose(wcfClient *wcli);
/*!
 * \brief サーバーへの接続を別スレッドで開始する。
 * \since ver1.5
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfWaitConnection(), wcfAutoReconnect()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfStart(wcfClient *wcli);
/*!
 * \brief サーバーへの接続を別スレッドで開始し、成功するまで待機する。
 * \since ver2.0
 *
 * * wcfAutoReconnect が無効の場合は1回目の接続のみ待機し、
 * 失敗しても再接続せずreturnする。
 * * autoRecvが無効の場合、初期化が完了するまで
 * wcfRecv() をこのスレッドで呼び出す。
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfStart(), wcfAutoReconnect()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfWaitConnection(wcfClient *wcli);
/*!
 * \brief 通信が切断されたときに自動で再試行するかどうかを設定する。
 * \since ver1.11.1
 * \param wcli
 * \param enabled 0以外にすると有効、0にすると無効になる。デフォルトは有効
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfStart()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfAutoReconnect(wcfClient *wcli,
                                                      int enabled);
/*!
 * \brief サーバーからデータを受信する
 * \since ver2.0
 *
 * * データを受信した場合、各種コールバック(onEntry, onChange,
 * Funcなど)をこのスレッドで呼び出し、
 * それがすべて完了するまでこの関数はブロックされる。
 * * データを何も受信しなかった場合、サーバーに接続していない場合、
 * または接続試行中やデータ送信中など受信ができない場合は、
 * 即座にreturnする。
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfWaitRecvFor(), wcfWaitRecv(), wcfAutoRecv()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfRecv(wcfClient *wcli);
/*!
 * \brief サーバーからデータを受信する
 * \since ver2.0
 *
 * * wcfRecv()と同じだが、何も受信できなければ
 * timeout 経過後に再試行してreturnする。
 * timeout=0 または負の値なら再試行せず即座にreturnする。(wcfRecv()と同じ)
 * * timeoutが100μs以上の場合、100μsおきに繰り返し再試行し、timeout経過後return
 *
 * \param wcli
 * \param timeout (μs単位)
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfRecv(), wcfWaitRecv(), wcfAutoRecv()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfWaitRecvFor(wcfClient *wcli,
                                                    int timeout);
/*!
 * \brief サーバーからデータを受信する
 * \since ver2.0
 *
 * * wcfRecv()と同じだが、何か受信するまで無制限に待機する
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfRecv(), wcfAutoRecv()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfWaitRecv(wcfClient *wcli);
/*!
 * \brief 別スレッドでwcfRecv()を自動的に呼び出すようにする。
 * \since ver2.0
 *
 * * wcfStart() や wcfWaitConnection() より前に設定する必要がある。
 * * autoRecvが有効の場合、別スレッドで一定間隔(100μs)ごとにrecv()が呼び出され、
 * 各種コールバック (onEntry, onChange, Funcなど)
 * も別のスレッドで呼ばれることになる
 * (そのためmutexなどを適切に設定すること)
 * * デフォルトでは無効なので、手動でwcfRecv()などを呼び出す必要がある
 *
 * \sa wcfRecv(), wcfWaitRecvFor(), wcfWaitRecv()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfAutoRecv(wcfClient *wcli);
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
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfSync(wcfClient *wcli);

/*!
 * \brief wcfの関数から取得したポインタのデータを破棄
 * \since ver1.7
 * \param ptr データを格納したポインタ
 * \return ptrが wcfFuncRun, wcfFuncGetResult, wcfFuncWaitResult, wcfViewGet
 * で取得したものでない場合WCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfDestroy(const void *ptr);

#ifdef __cplusplus
}
#endif
