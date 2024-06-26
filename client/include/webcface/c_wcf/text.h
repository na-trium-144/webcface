#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief 文字列を送信する(null終端)
 * \since ver1.7
 * \param wcli Clientポインタ
 * \param field textの名前
 * \param text 送信する文字列(null終端)
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfTextSet(wcfClient *wcli, const char *field,
                                  const char *text);
/*!
 * \brief 文字列を送信する(null終端, wstring)
 * \since ver2.0
 * \sa wcfTextSet
 */
WEBCFACE_DLL wcfStatus wcfTextSetW(wcfClient *wcli, const wchar_t *field,
                                  const wchar_t *text);
/*!
 * \brief 文字列を送信する
 * \since ver1.7
 * \param wcli Clientポインタ
 * \param field textの名前
 * \param text 送信する文字列
 * \param size 送信する文字列の長さ
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfTextSetN(wcfClient *wcli, const char *field,
                                   const char *text, int size);
/*!
 * \brief 文字列を送信する (wstring)
 * \since ver2.0
 * \sa wcfTextSetN
 */
WEBCFACE_DLL wcfStatus wcfTextSetNW(wcfClient *wcli, const wchar_t *field,
                                   const wchar_t *text, int size);

/*!
 * \brief 文字列を受信する
 * \since ver1.7
 *
 * sizeに指定したサイズより実際に受信した文字列の長さのほうが大きいか同じ場合、
 * textには(size-1)文字+null終端を格納しrecv_sizeには本来の長さを返す
 *
 * size > recv_size の場合、またはWCF_NOT_FOUNDの場合、
 * null終端より後ろの余った範囲はそのまま
 *
 * \param wcli Clientポインタ
 * \param member memberの名前 (ver1.7〜:NULLまたは空文字列で自分自身を指す)
 * \param field textの名前
 * \param text 受信した文字列を格納するポインタ
 * \param size 配列のサイズ
 * \param recv_size 実際に受信した文字列の長さが返る
 * \return wcliが無効ならWCF_BAD_WCLI,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND
 *
 */
WEBCFACE_DLL wcfStatus wcfTextGet(wcfClient *wcli, const char *member,
                                  const char *field, char *text, int size,
                                  int *recv_size);
/*!
 * \brief 文字列を受信する (wstring)
 * \since ver2.0
 * \sa wcfTextGet
 */
WEBCFACE_DLL wcfStatus wcfTextGetW(wcfClient *wcli, const wchar_t *member,
                                  const wchar_t *field, wchar_t *text, int size,
                                  int *recv_size);

#ifdef __cplusplus
}
#endif
