#pragma once
#include "../common/def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void wcfClient;
typedef int wcfStatus;
typedef int wcfValType;
typedef void wcfAsyncFuncResult;

#define WCF_OK 0
#define WCF_BAD_WCLI 1
#define WCF_BAD_HANDLE 2
#define WCF_INVALID_ARGUMENT 3
#define WCF_NOT_FOUND 4
#define WCF_EXCEPTION 5
#define WCF_NOT_CALLED 6
#define WCF_NOT_RETURNED 7

#define WCF_VAL_NONE 0
#define WCF_VAL_STRING 1
#define WCF_VAL_BOOL 2
#define WCF_VAL_INT 3
#define WCF_VAL_FLOAT 4
#define WCF_VAL_DOUBLE 4

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

/*!
 * \brief 受信した関数呼び出しの情報を保持するstruct
 *
 */
WEBCFACE_DLL typedef struct wcfFuncCallHandle {
    /*!
     * \brief 呼び出された引数
     *
     */
    const wcfMultiVal *args;
    /*!
     * \brief 引数の個数
     *
     * listen時に指定した個数と必ず同じになる。
     *
     */
    int arg_size;
} wcfFuncCallHandle;

#ifdef __cplusplus
}
#endif
