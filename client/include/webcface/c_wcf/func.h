#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief int型のwcfMultiValを構築
 * \since ver1.5
 */
WEBCFACE_DLL wcfMultiVal WEBCFACE_CALL wcfValI(int value);
/*!
 * \brief int型のwcfMultiValWを構築
 * \since ver2.0
 */
WEBCFACE_DLL wcfMultiValW WEBCFACE_CALL wcfValWI(int value);
/*!
 * \brief double型のwcfMultiValを構築
 * \since ver1.5
 */
WEBCFACE_DLL wcfMultiVal WEBCFACE_CALL wcfValD(double value);
/*!
 * \brief double型のwcfMultiValWを構築
 * \since ver2.0
 */
WEBCFACE_DLL wcfMultiValW WEBCFACE_CALL wcfValWD(double value);
/*!
 * \brief 文字列型のwcfMultiValを構築
 * \since ver1.5
 */
WEBCFACE_DLL wcfMultiVal WEBCFACE_CALL wcfValS(const char *value);
/*!
 * \brief 文字列型のwcfMultiValWを構築 (wstring)
 * \since ver2.0
 */
WEBCFACE_DLL wcfMultiValW WEBCFACE_CALL wcfValWS(const wchar_t *value);

/*!
 * \brief 関数を呼び出す
 * \since ver1.5
 * \param wcli Clientポインタ
 * \param member memberの名前 (ver1.7〜:NULLまたは空文字列で自分自身を指す)
 * \param field funcの名前
 * \param args 引数の配列
 * \param arg_size 引数の個数
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return wcliが無効ならwcfBadClient,
 * 対象のmemberやfieldが存在しない場合 wcfNotFound,
 * 関数で例外が発生した場合 wcfException
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncRun(wcfClient *wcli, const char *member, const char *field,
           const wcfMultiVal *args, int arg_size, wcfMultiVal **result);
/*!
 * \brief 関数を呼び出す (wstring)
 * \since ver2.0
 * \sa wcfFuncRun
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncRunW(wcfClient *wcli, const wchar_t *member, const wchar_t *field,
            const wcfMultiValW *args, int arg_size, wcfMultiValW **result);

/*!
 * \brief 関数を非同期で呼び出す
 * \since ver1.5
 * \param wcli Clientポインタ
 * \param member memberの名前 (ver1.7〜:NULLまたは空文字列で自分自身を指す)
 * \param field funcの名前
 * \param args 引数の配列
 * \param arg_size 引数の個数
 * \param async_res 結果を格納する変数(wcfAsyncFuncResult*)へのポインタ
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncRunAsync(
    wcfClient *wcli, const char *member, const char *field,
    const wcfMultiVal *args, int arg_size, wcfAsyncFuncResult **async_res);
/*!
 * \brief 関数を非同期で呼び出す (wstring)
 * \since ver2.0
 * \sa wcfFuncRunAsync
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncRunAsyncW(
    wcfClient *wcli, const wchar_t *member, const wchar_t *field,
    const wcfMultiValW *args, int arg_size, wcfAsyncFuncResult **async_res);

/*!
 * \brief 非同期で呼び出した関数の実行結果を取得
 * \since ver1.5
 * \param async_res 関数呼び出しに対応するAsyncFuncResult
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return async_resが無効な場合 wcfBadHandle,
 * 対象のmemberやfieldが存在しない場合 wcfNotFound,
 * 関数で例外が発生した場合 wcfException,
 * まだ結果が返ってきていない場合 wcfNotReturned
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncGetResult(wcfAsyncFuncResult *async_res, wcfMultiVal **result);
/*!
 * \brief 非同期で呼び出した関数の実行結果を取得 (wstring)
 * \since ver2.0
 * \sa wcfFuncGetResult
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncGetResultW(wcfAsyncFuncResult *async_res, wcfMultiValW **result);
/*!
 * \brief 非同期で呼び出した関数の実行完了まで待機し、結果を取得
 * \since ver1.5
 * \param async_res 関数呼び出しに対応するAsyncFuncResult
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return async_resが無効な場合 wcfBadHandle,
 * 対象のmemberやfieldが存在しない場合 wcfNotFound,
 * 関数で例外が発生した場合 wcfException
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncWaitResult(wcfAsyncFuncResult *async_res, wcfMultiVal **result);
/*!
 * \brief 非同期で呼び出した関数の実行完了まで待機し、結果を取得 (wstring)
 * \since ver2.0
 * \sa wcfFuncWaitResult
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncWaitResultW(wcfAsyncFuncResult *async_res, wcfMultiValW **result);

/*!
 * \brief 関数を登録する
 * \since ver1.9
 *
 * * 登録した関数は引数でcallhandleを受け取り、
 * wcfFuncRespond または wcfFuncReject を使って結果を返す。
 * (何も返さずreturnしても良いが、return後にはそのhandleは使えない)
 * * (ver2.0〜) wcfFuncSet()でセットした場合、他クライアントから呼び出されたとき
 * wcfRecv() (または autoRecv) のスレッドでそのまま実行され、
 * この関数が完了するまで他のデータの受信はブロックされる。
 * また、 wcfFuncRunAsync() で呼び出したとしても同じスレッドで同期実行される。
 *
 * \param wcli Clientポインタ
 * \param field 関数名
 * \param arg_types 受け取る引数の型をwcfValTypeの配列で指定
 * \param arg_size 受け取る引数の個数
 * \param return_type 戻り値の型を指定
 * \param callback 実行する関数:
 * wcfFuncCallhandle* 型と void* 型の引数を1つずつ取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第2引数にそのまま渡される。
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncSet(wcfClient *wcli, const char *field, const wcfValType *arg_types,
           int arg_size, wcfValType return_type, wcfFuncCallback callback,
           void *user_data);
/*!
 * \brief 関数を登録する (wstring)
 * \since ver2.0
 * \sa wcfFuncSet
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncSetW(wcfClient *wcli, const wchar_t *field, const wcfValType *arg_types,
            int arg_size, wcfValType return_type, wcfFuncCallbackW callback,
            void *user_data);
/*!
 * \brief 非同期に実行される関数を登録する
 * \since ver2.0
 *
 * 登録した関数は他クライアントから呼び出されたとき新しいスレッドを建てて実行される。
 * ver1.11以前のwcfFuncSet()と同じ。
 *
 * \sa wcfFuncSet
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncSetAsync(wcfClient *wcli, const char *field, const wcfValType *arg_types,
                int arg_size, wcfValType return_type, wcfFuncCallback callback,
                void *user_data);
/*!
 * \brief 非同期に実行される関数を登録する (wstring)
 * \since ver2.0
 * \sa wcfFuncSetW, wcfFuncSetAsync
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncSetAsyncW(
    wcfClient *wcli, const wchar_t *field, const wcfValType *arg_types,
    int arg_size, wcfValType return_type, wcfFuncCallbackW callback,
    void *user_data);

/*!
 * \brief 関数呼び出しの待受を開始する
 * \since ver1.5
 * \param wcli Clientポインタ
 * \param field 関数名
 * \param arg_types 受け取る引数の型をwcfValTypeの配列で指定
 * \param arg_size 受け取る引数の個数
 * \param return_type 戻り値の型を指定
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncListen(wcfClient *wcli,
                                                   const char *field,
                                                   const wcfValType *arg_types,
                                                   int arg_size,
                                                   wcfValType return_type);
/*!
 * \brief 関数呼び出しの待受を開始する (wstring)
 * \since ver2.0
 * \sa wcfFuncListen
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncListenW(wcfClient *wcli,
                                                    const wchar_t *field,
                                                    const wcfValType *arg_types,
                                                    int arg_size,
                                                    wcfValType return_type);
/*!
 * \brief 関数が呼び出されたかどうかを確認
 * \since ver1.5
 *
 * 1回の関数呼び出しに対してfetchCallは1回だけhandlerを返す
 *
 * 呼び出されたらその引数と、値を返す用の関数が入ったhandlerを返す。
 * まだ呼び出されてなければnulloptを返す。
 *
 * \param wcli Clientポインタ
 * \param field 関数名
 * \param handle handleポインタを受け取る変数
 * \return wcliが無効ならwcfBadClient,
 * まだ関数が呼び出されていない or ListenしていないならwcfNotCalled
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncFetchCall(
    wcfClient *wcli, const char *field, wcfFuncCallHandle **handle);
/*!
 * \brief 関数が呼び出されたかどうかを確認 (wstring)
 * \since ver2.0
 * \sa wcfFuncFetchCall
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncFetchCallW(
    wcfClient *wcli, const wchar_t *field, wcfFuncCallHandleW **handle);

/*!
 * \brief 関数呼び出しに対して値を返す
 * \since ver1.5
 *
 * 値を返すとhandleはdeleteされ使えなくなる
 * \param handle 関数呼び出しに対応するhandle
 * \param value 返す値 (ver1.9〜 NULLも可)
 * \return handleが無効ならwcfBadHandle
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncRespond(const wcfFuncCallHandle *handle, const wcfMultiVal *value);
/*!
 * \brief 関数呼び出しに対して値を返す (wstring)
 * \since ver2.0
 * \sa wcfFuncRespond
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncRespondW(const wcfFuncCallHandleW *handle, const wcfMultiValW *value);
/*!
 * \brief 関数呼び出しに対してエラーメッセージを返す
 * \since ver1.5
 *
 * エラーメッセージを返すとhandleはdeleteされ使えなくなる
 * \param handle 関数呼び出しに対応するhandle
 * \param message 返すメッセージ (空文字列の代わりにNULLも可)
 * \return handleが無効ならwcfBadHandle
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncReject(const wcfFuncCallHandle *handle, const char *message);
/*!
 * \brief 関数呼び出しに対してエラーメッセージを返す (wstring)
 * \since ver2.0
 * \sa wcfFuncReject
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncRejectW(const wcfFuncCallHandleW *handle, const wchar_t *message);

#ifdef __cplusplus
}
#endif
