#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

WEBCFACE_DLL wcfMultiVal wcfValI(int value);
WEBCFACE_DLL wcfMultiVal wcfValD(double value);
WEBCFACE_DLL wcfMultiVal wcfValS(const char *value);

/*!
 * \brief 関数を呼び出す
 *
 * \param wcli Clientポインタ
 * \param member memberの名前
 * \param field funcの名前
 * \param args 引数の配列
 * \param arg_size 引数の個数
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return wcliが無効ならWCF_BAD_WCLI,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND,
 * 関数で例外が発生した場合 WCF_EXCEPTION
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncRun(wcfClient *wcli, const char *member,
                                  const char *field, const wcfMultiVal *args,
                                  int arg_size, wcfMultiVal **result);

/*!
 * \brief 関数を非同期で呼び出す
 *
 * \param wcli Clientポインタ
 * \param member memberの名前
 * \param field funcの名前
 * \param args 引数の配列
 * \param arg_size 引数の個数
 * \param async_res 結果を格納する変数(wcfAsyncFuncResult*)へのポインタ
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncRunAsync(wcfClient *wcli, const char *member,
                                       const char *field,
                                       const wcfMultiVal *args, int arg_size,
                                       wcfAsyncFuncResult **async_res);

/*!
 * \brief 非同期で呼び出した関数の実行結果を取得
 *
 * \param async_res 関数呼び出しに対応するAsyncFuncResult
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return async_resが無効な場合 WCF_BAD_HANDLE,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND,
 * 関数で例外が発生した場合 WCF_EXCEPTION,
 * まだ結果が返ってきていない場合 WCF_NOT_RETURNED
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncGetResult(wcfAsyncFuncResult *async_res,
                                        wcfMultiVal **result);
/*!
 * \brief 非同期で呼び出した関数の実行完了まで待機し、結果を取得
 *
 * \param async_res 関数呼び出しに対応するAsyncFuncResult
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return async_resが無効な場合 WCF_BAD_HANDLE,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND,
 * 関数で例外が発生した場合 WCF_EXCEPTION
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncWaitResult(wcfAsyncFuncResult *async_res,
                                         wcfMultiVal **result);

/*!
 * \brief 取得した関数の戻り値のデータを破棄
 *
 * \param result 結果を格納した変数(wcfMultiVal*)
 * \return resultが wcfFuncRun, wcfFuncGetResult, wcfFuncWaitResult
 * で取得したものでない場合WCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncFreeResult(const wcfMultiVal *result);

/*!
 * \brief 関数呼び出しの待受を開始する
 *
 * \param wcli Clientポインタ
 * \param field 関数名
 * \param arg_types 受け取る引数の型をwcfValTypeの配列で指定
 * \param arg_size 受け取る引数の個数
 * \param return_type 戻り値の型を指定
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncListen(wcfClient *wcli, const char *field,
                                     const wcfValType *arg_types, int arg_size,
                                     wcfValType return_type);
/*!
 * \brief 関数が呼び出されたかどうかを確認
 *
 * 1回の関数呼び出しに対してfetchCallは1回だけhandlerを返す
 *
 * 呼び出されたらその引数と、値を返す用の関数が入ったhandlerを返す。
 * まだ呼び出されてなければnulloptを返す。
 *
 * \param wcli Clientポインタ
 * \param field 関数名
 * \param handle handleポインタを受け取る変数
 * \return wcliが無効ならWCF_BAD_WCLI,
 * まだ関数が呼び出されていない or ListenしていないならWCF_NOT_CALLED
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncFetchCall(wcfClient *wcli, const char *field,
                                        wcfFuncCallHandle **handle);

/*!
 * \brief 関数呼び出しに対して値を返す
 *
 * 値を返すとhandleはdeleteされ使えなくなる
 * \param handle 関数呼び出しに対応するhandle
 * \param value 返す値
 * \return handleが無効ならWCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncRespond(const wcfFuncCallHandle *handle,
                                      const wcfMultiVal *value);
/*!
 * \brief 関数呼び出しに対してエラーメッセージを返す
 *
 * エラーメッセージを返すとhandleはdeleteされ使えなくなる
 * \param handle 関数呼び出しに対応するhandle
 * \param message 返すメッセージ
 * \return handleが無効ならWCF_BAD_HANDLE
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncReject(const wcfFuncCallHandle *handle,
                                     const char *message);

#ifdef __cplusplus
}
#endif
