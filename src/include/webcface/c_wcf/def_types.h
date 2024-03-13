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

typedef void (*wcfFuncCallback)(wcfFuncCallHandle *call_handle);

#define WCF_VIEW_TEXT 0
#define WCF_VIEW_NEW_LINE 1
#define WCF_VIEW_BUTTON 2

#define WCF_COLOR_INHERIT 0
#define WCF_COLOR_BLACK 1
#define WCF_COLOR_WHITE 2
#define WCF_COLOR_GRAY 4
#define WCF_COLOR_RED 8
#define WCF_COLOR_ORANGE 9
#define WCF_COLOR_YELLOW 11
#define WCF_COLOR_GREEN 13
#define WCF_COLOR_TEAL 15
#define WCF_COLOR_CYAN 16
#define WCF_COLOR_BLUE 18
#define WCF_COLOR_INDIGO 19
#define WCF_COLOR_PURPLE 21
#define WCF_COLOR_PINK 23

WEBCFACE_DLL typedef struct wcfViewComponent {
    /*!
     * \brief Componentの種類
     *
     */
    int type;
    /*!
     * \brief 表示する文字列 (空の場合nullptr)
     *
     */
    const char *text;
    /*!
     * \brief クリック時に実行するFuncのmemberとfield、またはnullptr
     *
     */
    const char *on_click_member, *on_click_field;
    /*!
     * \brief テキストの色
     *
     */
    int text_color;
    /*!
     * \brief 背景の色
     *
     */
    int bg_color;
} wcfViewComponent;

#ifdef __cplusplus
}
#endif
