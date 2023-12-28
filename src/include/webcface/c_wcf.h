#pragma once
#include "common/def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void wcfClient;
typedef int wcfStatus;
typedef int wcfValType;

#define WCF_OK 0
#define WCF_BAD_WCLI 1
#define WCF_BAD_HANDLE 2
#define WCF_INVALID_ARGUMENT 3
#define WCF_NOT_FOUND 4
#define WCF_EXCEPTION 5
#define WCF_NOT_CALLED 6

#define WCF_VAL_NONE 0
#define WCF_VAL_STRING 1
#define WCF_VAL_BOOL 2
#define WCF_VAL_INT 3
#define WCF_VAL_FLOAT 4
#define WCF_VAL_DOUBLE 4

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

/*!
 * \brief 単一の値を送信する
 * \param wcli Clientポインタ
 * \param field valueの名前
 * \param value 送信する値
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfValueSet(wcfClient *wcli, const char *field,
                                   double value);
/*!
 * \brief 複数の値を送信する(doubleの配列)
 * \param wcli Clientポインタ
 * \param field valueの名前
 * \param values 送信する値の配列の先頭のポインタ
 * \param size 送信する値の数
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfValueSetVecD(wcfClient *wcli, const char *field,
                                       const double *values, int size);

/*!
 * \brief 値を受信する
 *
 * sizeに指定したサイズより実際に受信した値の個数のほうが大きい場合、
 * valuesにはsize分の値のみを格納しrecv_sizeには本来のサイズを返す
 *
 * size > recv_size の場合、またはWCF_NOT_FOUNDの場合、
 * 配列の余った範囲は0で埋められる
 *
 * \param wcli Clientポインタ
 * \param member memberの名前
 * \param field valueの名前
 * \param values 受信した値を格納する配列へのポインタ
 * \param size 配列のサイズ
 * \param recv_size 実際に受信した値の個数が返る
 * \return wcliが無効ならWCF_BAD_WCLI,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND
 *
 */
WEBCFACE_DLL wcfStatus wcfValueGetVecD(wcfClient *wcli, const char *member,
                                       const char *field, double *values,
                                       int size, int *recv_size);

/*!
 * \brief 数値と文字列をまとめて扱うためのstruct
 *
 * wcfMultiValを引数に渡す場合は、 as_int, as_double, as_str
 * のいずれか1つのみに値を入れて使う。
 * wcfValI(), wcfValD(), wcfValS() 関数で値をセットできる。
 *
 * wcfMultiValが関数から返ってくる場合は、as_int, as_double,
 * as_strがすべて埋まった状態で返ってくる。
 *
 */
WEBCFACE_DLL typedef struct wcfMultiVal {
    /*!
     * \brief int型でのアクセス
     *
     */
    int as_int;
    /*!
     * \brief double型でのアクセス
     *
     */
    double as_double;
    /*!
     * \brief char*型でのアクセス
     *
     * as_intまたはas_doubleに値をセットして渡す場合はas_strはnullにすること。
     *
     * 値が返ってくる場合はas_strがnullになっていることはない。(何も返さない場合でも空文字列が入る)
     *
     */
    const char *as_str;
} wcfMultiVal;

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
 * \brief 受信した関数呼び出しの情報を保持するstruct
 *
 */
WEBCFACE_DLL typedef struct wcfFuncCallHandle {
    /*!
     * \brief 呼び出された引数
     *
     */
    const wcfMultiVal *const args;
    /*!
     * \brief 引数の個数
     *
     * listen時に指定した個数と必ず同じになる。
     *
     */
    const int arg_size;
    void *const handle;
} wcfFuncCallHandle;

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
