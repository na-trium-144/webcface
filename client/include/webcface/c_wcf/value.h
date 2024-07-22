#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief 単一の値を送信する
 * \since ver1.5
 * \param wcli Clientポインタ
 * \param field valueの名前
 * \param value 送信する値
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueSet(wcfClient *wcli,
                                                 const char *field,
                                                 double value);
/*!
 * \brief 単一の値を送信する (wstring)
 * \since ver2.0
 * \sa wcfValueSet
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueSetW(wcfClient *wcli,
                                                  const wchar_t *field,
                                                  double value);
/*!
 * \brief 複数の値を送信する(doubleの配列)
 * \since ver1.5
 * \param wcli Clientポインタ
 * \param field valueの名前
 * \param values 送信する値の配列の先頭のポインタ
 * \param size 送信する値の数
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueSetVecD(wcfClient *wcli,
                                                     const char *field,
                                                     const double *values,
                                                     int size);
/*!
 * \brief 複数の値を送信する(doubleの配列, wstring)
 * \since ver2.0
 * \sa wcfValueSetVecD
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueSetVecDW(wcfClient *wcli,
                                                      const wchar_t *field,
                                                      const double *values,
                                                      int size);

/*!
 * \brief 値を受信する
 * \since ver1.7
 *
 * 配列データを受信した場合先頭の1つのみを返す
 *
 * wcfNotFoundの場合valueには0が返る
 * \param wcli Clientポインタ
 * \param member memberの名前 (ver1.7〜:NULLまたは空文字列で自分自身を指す)
 * \param field valueの名前
 * \param value 受信した値が返る
 * \return wcliが無効ならwcfBadClient,
 * ~~まだ値を受信していない場合 wcfNotFound~~,
 * (ver2.0〜)まだ値を受信していない場合 wcfNoData
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueGet(wcfClient *wcli,
                                                 const char *member,
                                                 const char *field,
                                                 double *value);
/*!
 * \brief 値を受信する (wstring)
 * \since ver2.0
 * \sa wcfValueGet
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueGetW(wcfClient *wcli,
                                                  const wchar_t *member,
                                                  const wchar_t *field,
                                                  double *value);
/*!
 * \brief 値を受信する
 * \since ver1.5
 *
 * sizeに指定したサイズより実際に受信した値の個数のほうが大きい場合、
 * valuesにはsize分の値のみを格納しrecv_sizeには本来のサイズを返す
 *
 * size > recv_size の場合、またはwcfNotFoundの場合、
 * 配列の余った範囲は0で埋められる
 *
 * \param wcli Clientポインタ
 * \param member memberの名前 (ver1.7〜:NULLまたは空文字列で自分自身を指す)
 * \param field valueの名前
 * \param values 受信した値を格納する配列へのポインタ
 * (ver2.0〜:size=0ならNULLも可)
 * \param size 配列のサイズ
 * \param recv_size 実際に受信した値の個数が返る
 * \return wcliが無効ならwcfBadClient,
 * ~~まだ値を受信していない場合 wcfNotFound~~,
 * (ver2.0〜)まだ値を受信していない場合 wcfNoData
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueGetVecD(wcfClient *wcli,
                                                     const char *member,
                                                     const char *field,
                                                     double *values, int size,
                                                     int *recv_size);
/*!
 * \brief 値を受信する (wstring)
 * \since ver2.0
 * \sa wcfValueGetVecD
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueGetVecDW(wcfClient *wcli,
                                                      const wchar_t *member,
                                                      const wchar_t *field,
                                                      double *values, int size,
                                                      int *recv_size);

/*!
 * \brief Valueが変化した時のイベント
 * \since ver2.0
 * \param wcli
 * \param member member名 (NULLまたは空文字列で自分自身を指す)
 * \param field valueの名前
 * \param callback 実行する関数:
 * const char* 型2つ(Memberとfieldの名前が渡される)と void*
 * 型の引数1つを取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第3引数にそのまま渡される。
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfValueChangeEvent(wcfClient *wcli, const char *member, const char *field,
                    wcfEventCallback2 callback, void *user_data);
/*!
 * \brief Valueが変化した時のイベント (wstring)
 * \since ver2.0
 * \sa wcfValueChangeEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueChangeEventW(
    wcfClient *wcli, const wchar_t *member, const wchar_t *field,
    wcfEventCallback2W callback, void *user_data);

#ifdef __cplusplus
}
#endif
