#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 * 配列データを受信した場合先頭の1つのみを返す
 *
 * WCF_NOT_FOUNDの場合valueには0が返る
 * \param wcli Clientポインタ
 * \param member memberの名前
 * \param field valueの名前
 * \param value 受信した値が返る
 * \return wcliが無効ならWCF_BAD_WCLI,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND
 *
 */
WEBCFACE_DLL wcfStatus wcfValueGet(wcfClient *wcli, const char *member,
                                   const char *field, double *value);
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

#ifdef __cplusplus
}
#endif
