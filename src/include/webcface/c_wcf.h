#pragma once
#include "common/def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *wcfClient;

enum wcfStatus {
    WCF_OK = 0,
    WCF_BAD_WCLI = 1,
    WCF_NOT_FOUND = 2,
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
 * \return wcliが正常にwcfInitされwcfCloseする前のポインタであれば1、そうでなければ0
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
// WEBCFACE_DLL int wcfValueSetVecI(void *wcli, const char *field, const int
// *values, int size);
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

WEBCFACE_DLL const wcfMultiVal *wcfFuncRun(wcfClient wcli, const char *member,
                                           const char *field, wcfMultiVal *args,
                                           int arg_size);

WEBCFACE_DLL const char *wcfFuncRunS(wcfClient wcli, const char *member,
                                     const char *field, const char **args,
                                     int arg_size);

#ifdef __cplusplus
}
#endif
