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
 * * 接続だけでなくentryの受信や初期化が完了するまで待機する。
 * wcfWaitSync() と同様、このスレッドで受信処理
 * (EntryEvent コールバックの呼び出しなど) が行われる。
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
 * \brief
 * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する
 * \since ver1.5
 *
 * * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
 * * サーバーに接続していない場合、wcfStart()を呼び出す。
 * * ver2.0以降: データを受信した場合、各種コールバック(onEntry, onChange,
 * Funcなど)をこのスレッドで呼び出し、
 * それがすべて完了するまでこの関数はブロックされる。
 *   * データを何も受信しなかった場合、サーバーに接続していない場合、
 * または接続試行中やデータ送信中など受信ができない場合は、
 * 即座にreturnする。
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfWaitSyncFor(), wcfWaitSync()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfSync(wcfClient *wcli);
/*!
 * \brief
 * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する
 * \since ver2.0
 *
 * * wcfSync()と同じだが、何も受信できなければ
 * timeout 経過後に再試行してreturnする。
 * timeout=0 または負の値なら再試行せず即座にreturnする。(wcfSync()と同じ)
 * * timeoutが100μs以上の場合、100μsおきに繰り返し再試行し、timeout経過後return
 *
 * \param wcli
 * \param timeout (μs単位)
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfSync(), wcfWaitSync()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfWaitSyncFor(wcfClient *wcli,
                                                    int timeout);
/*!
 * \brief
 * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する
 * \since ver2.0
 *
 * * wcfSync()と同じだが、何か受信するまで無制限に待機する
 *
 * \return wcliが無効ならWCF_BAD_WCLI
 * \sa wcfSync()
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfWaitSync(wcfClient *wcli);
// /*!
//  * \brief 別スレッドでwcfSync()を自動的に呼び出すようにする。
//  * \since ver2.0
//  *
//  * * wcfStart() や wcfWaitConnection() より前に設定する必要がある。
//  * * autoSyncが有効の場合、別スレッドで一定間隔(100μs)ごとにwcfSync()が呼び出され、
//  * 各種コールバック (onEntry, onChange, Funcなど)
//  * も別のスレッドで呼ばれることになる
//  * (そのためmutexなどを適切に設定すること)
//  * * デフォルトでは無効なので、手動でwcfSync()などを呼び出す必要がある
//  *
//  * \param wcli
//  * \param enabled 0以外にすると有効、0にすると無効になる。デフォルトは無効
//  * \sa wcfSync(), wcfWaitSyncFor(), wcfWaitSync()
//  */
// WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfAutoSync(wcfClient *wcli, int enabled);

/*!
 * \brief wcfの関数から取得したポインタのデータを破棄
 * \since ver1.7
 * \param ptr データを格納したポインタ
 * \return ptrが wcfFuncRun, wcfFuncGetResult, wcfFuncWaitResult, wcfViewGet
 * で取得したものでない場合WCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfDestroy(const void *ptr);

/*!
 * \brief サーバーに接続されている他のmemberのリストを得る。
 * \since ver2.0
 *
 * * 自分自身と、無名のmemberを除く。
 * * sizeに指定したサイズよりmemberの数が多い場合、
 * size分のmember名を格納する。
 * * sizeに指定したサイズよりmemberの数が少ない場合、
 * size分より後ろの余った部分はそのまま
 *
 * \param wcli
 * \param list member名を格納するchar*の配列
 * (size=0ならNULLも可)
 * \param size listの要素数
 * \param members_num 実際のmember数
 * \return wcliが無効ならWCF_BAD_WCLI
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfMemberList(wcfClient *wcli,
                                                   const char **list, int size,
                                                   int *members_num);
/*!
 * \brief サーバーに接続されている他のmemberのリストを得る。
 * \since ver2.0
 * \sa wcfMemberList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfMemberListW(wcfClient *wcli,
                                                    const wchar_t **list,
                                                    int size, int *members_num);
/*!
 * \brief Memberが追加された時のイベント
 * \since ver2.0
 * \param wcli
 * \param callback 実行する関数:
 * const char* 型(追加されたMemberの名前が渡される)と void*
 * 型の引数を1つずつ取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第2引数にそのまま渡される。
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfMemberEntryEvent(
    wcfClient *wcli, wcfEventCallback1 callback, void *user_data);
/*!
 * \brief Memberが追加された時のイベント (wstring)
 * \since ver2.0
 * \sa wcfMemberEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfMemberEntryEventW(
    wcfClient *wcli, wcfEventCallback1W callback, void *user_data);

/*!
 * \brief WebCFaceサーバーのバージョン情報を返す
 * \since ver2.0
 * \return サーバーのバージョンを表す文字列、またはwcliが無効な場合空文字列
 *
 */
WEBCFACE_DLL const char *WEBCFACE_CALL wcfServerVersion(wcfClient *wcli);
/*!
 * \brief WebCFaceサーバーの識別情報を返す
 * \since ver2.0
 * \return サーバーの識別情報を表す文字列(通常は"webcface")、
 * またはwcliが無効な場合空文字列
 *
 */
WEBCFACE_DLL const char *WEBCFACE_CALL wcfServerName(wcfClient *wcli);
/*!
 * \brief WebCFaceサーバーのホスト名を返す
 * \since ver2.0
 * \return サーバーのホスト名、
 * またはwcliが無効な場合空文字列
 *
 */
WEBCFACE_DLL const char *WEBCFACE_CALL wcfServerHostName(wcfClient *wcli);

#ifdef __cplusplus
}
#endif
