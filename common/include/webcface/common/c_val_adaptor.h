#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_C_BEGIN

/*!
 * \brief 引数や戻り値の型を表すenum
 *
 */
typedef enum wcfValType {
    WCF_VAL_NONE = 0,
    WCF_VAL_STRING = 1,
    WCF_VAL_BOOL = 2,
    WCF_VAL_INT = 3,
    WCF_VAL_DOUBLE = 4,
} wcfValType;

/*!
 * \brief 数値と文字列をまとめて扱うためのstruct
 *
 * wcfMultiValを引数に渡す場合は、 as_int, as_double, as_str
 * のいずれか1つのみに値を入れて使う。
 * 手動で初期化するのではなく、wcfValI(), wcfValD(), wcfValS()
 * 関数を使うと値を適切にセットしたwcfMultiValを構築できる。
 *
 * wcfMultiValが関数から返ってくる場合は、as_int, as_double,
 * as_strがすべて埋まった状態で返ってくる。
 *
 */
typedef struct wcfMultiVal {
    /*!
     * \brief double型でのアクセス
     * \sa wcfValD
     */
    double as_double;
    /*!
     * \brief char*型でのアクセス
     *
     * * as_int, as_double に値をセットして渡す場合は
     * as_str はnullにすること。
     * * 値が返ってくる場合はas_strがnullになっていることはない。
     * (何も返さない場合でも空文字列が入る)
     * \sa wcfValS
     */
    const char *as_str;
    /*!
     * \brief int型でのアクセス
     * \sa wcfValI
     */
    int as_int;
} wcfMultiVal;
/*!
 * \brief 数値と文字列をまとめて扱うためのstruct (wstring)
 *
 * wcfMultiValWを引数に渡す場合は、 as_int, as_double, as_str
 * のいずれか1つのみに値を入れて使う。
 * 手動で初期化するのではなく、wcfValWI(), wcfValWD(), wcfValWS()
 * 関数を使うと値を適切にセットしたwcfMultiValを構築できる。
 *
 * wcfMultiValWが関数から返ってくる場合は、as_int, as_double,
 * as_strがすべて埋まった状態で返ってくる。
 *
 */
typedef struct wcfMultiValW {
    /*!
     * \brief double型でのアクセス
     * \sa wcfValWD
     */
    double as_double;
    /*!
     * \brief wchar_t*型でのアクセス
     *
     * * as_int, as_double に値をセットして渡す場合は
     * as_str はnullにすること。
     * * 値が返ってくる場合はas_strがnullになっていることはない。
     * (何も返さない場合でも空文字列が入る)
     * \sa wcfValWS
     */
    const wchar_t *as_str;
    /*!
     * \brief int型でのアクセス
     * \sa wcfValWI
     */
    int as_int;
} wcfMultiValW;

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

WEBCFACE_C_END
