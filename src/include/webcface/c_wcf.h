#pragma once
#include "common/def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *wcfClient;

enum wcfStatus : int {
    WCF_OK = 0,
    WCF_BAD_WCLI = 1,
    WCF_BAD_HANDLER = 2,
    WCF_INVALID_ARGUMENT = 3,
    WCF_NOT_FOUND = 4,
    WCF_EXCEPTION = 5,
    WCF_NOT_CALLED = 6,
};

enum wcfValType : int {
    WCF_NONE = 0,
    WCF_STRING = 1,
    WCF_BOOL = 2,
    WCF_INT = 3,
    WCF_FLOAT = 4,
};

/*!
 * \brief クライアントを初期化する
 * \return Clientのポインタ
 *
 */
WEBCFACE_DLL wcfClient wcfInit(const char *name, const char *host = "127.0.0.1",
                               int port = WEBCFACE_DEFAULT_PORT);
/*!
 * \brief 有効なClientのポインタであるかを返す
 * \return
 * wcliが正常にwcfInitされwcfCloseする前のポインタであれば1、そうでなければ0
 *
 */
WEBCFACE_DLL int wcfIsInstance(wcfClient wcli);
/*!
 * \brief クライアントを閉じる
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfClose(wcfClient wcli);
/*!
 * \brief 接続を開始する
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfStart(wcfClient wcli);
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
WEBCFACE_DLL wcfStatus wcfSync(wcfClient wcli);

/*!
 * \brief 単一の値を送信する
 * \param wcli Clientポインタ
 * \param field valueの名前
 * \param value 送信する値
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfValueSet(wcfClient wcli, const char *field,
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
WEBCFACE_DLL wcfStatus wcfValueSetVecD(wcfClient wcli, const char *field,
                                       const double *values, int size);

/*!
 * \brief 値を受信する
 *
 * sizeに指定したサイズより実際に受信した値の個数のほうが大きい場合、
 * valuesにはsize分の値のみを格納しrecv_sizeには本来のサイズを返す
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
WEBCFACE_DLL wcfStatus wcfValueGetVecD(wcfClient wcli, const char *member,
                                       const char *field, double *values,
                                       int size, int *recv_size);

typedef struct wcfMultiVal {
    int as_int;
    double as_double;
    const char *as_str;
} wcfMultiVal;

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
WEBCFACE_DLL wcfStatus wcfFuncRun(wcfClient wcli, const char *member,
                                  const char *field, const wcfMultiVal *args,
                                  int arg_size, wcfMultiVal **result);

/*!
 * \brief 関数を呼び出す(文字列配列)
 *
 * 数値やboolの引数は10進数で文字列にして渡す。
 *
 * \param wcli Clientポインタ
 * \param member memberの名前
 * \param field funcの名前
 * \param args 引数の文字列配列
 * \param arg_size 引数の個数
 * \param result 結果を格納する変数(wcfMultiVal*)へのポインタ
 * \return wcliが無効ならWCF_BAD_WCLI,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND,
 * 関数で例外が発生した場合 WCF_EXCEPTION
 *
 */
WEBCFACE_DLL wcfStatus wcfFuncRunS(wcfClient wcli, const char *member,
                                   const char *field, const char **args,
                                   int arg_size, wcfMultiVal **result);

/*!
 * \brief 関数を呼び出す(double配列)
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
WEBCFACE_DLL wcfStatus wcfFuncRunD(wcfClient wcli, const char *member,
                                   const char *field, const double *args,
                                   int arg_size, wcfMultiVal **result);

/*!
 * \brief 関数を呼び出す(int配列)
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
WEBCFACE_DLL wcfStatus wcfFuncRunI(wcfClient wcli, const char *member,
                                   const char *field, const int *args,
                                   int arg_size, wcfMultiVal **result);

struct wcfFuncListenerHandler {
    wcfMultiVal *args;
    int arg_size;
    void *handler;
};

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
WEBCFACE_DLL wcfStatus wcfFuncListen(wcfClient wcli, const char *field,
                                     const int *arg_types, int arg_size,
                                     int return_type);
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
 * \param handler handlerを受け取る変数のポインタを指定
 * \return wcliが無効ならWCF_BAD_WCLI,
 * まだ関数が呼び出されていない or ListenしていないならWCF_NOT_CALLED
 * 
 */
WEBCFACE_DLL wcfStatus wcfFuncFetchCall(wcfClient wcli, const char *field,
                                        wcfFuncListenerHandler **handler);

/*!
 * \brief 関数呼び出しに対して値を返す
 * 
 * \param handler 関数呼び出しに対応するhandler
 * \param value 返す値
 * \return handlerが無効またはすでにrespondかreject済みならWCF_BAD_HANDLER
 * 
 */
WEBCFACE_DLL wcfStatus wcfFuncRespond(const wcfFuncListenerHandler *handler,
                                      const wcfMultiVal *value);
/*!
 * \brief 関数呼び出しに対してエラーメッセージを返す
 * 
 * \param handler 関数呼び出しに対応するhandler
 * \param message 返すメッセージ
 * \return handlerが無効またはすでにrespondかreject済みならWCF_BAD_HANDLER
 * 
 */
WEBCFACE_DLL wcfStatus wcfFuncReject(const wcfFuncListenerHandler *handler,
                                     const char *message);

#ifdef __cplusplus
}
#endif
