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
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextSet(wcfClient *wcli,
                                                const char *field,
                                                const char *text);
/*!
 * \brief 文字列を送信する(null終端, wstring)
 * \since ver2.0
 * \sa wcfTextSet
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextSetW(wcfClient *wcli,
                                                 const wchar_t *field,
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
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextSetN(wcfClient *wcli,
                                                 const char *field,
                                                 const char *text, int size);
/*!
 * \brief 文字列を送信する (wstring)
 * \since ver2.0
 * \sa wcfTextSetN
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextSetNW(wcfClient *wcli,
                                                  const wchar_t *field,
                                                  const wchar_t *text,
                                                  int size);

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
 * (ver2.0〜:size=0ならNULLも可)
 * \param size 配列のサイズ
 * \param recv_size 実際に受信した文字列の長さが返る
 * \return wcliが無効ならWCF_BAD_WCLI,
 * ~~まだ値を受信していない場合 WCF_NOT_FOUND~~,
 * (ver2.0〜)まだ値を受信していない場合 WCF_NO_DATA
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextGet(wcfClient *wcli,
                                                const char *member,
                                                const char *field, char *text,
                                                int size, int *recv_size);
/*!
 * \brief 文字列を受信する (wstring)
 * \since ver2.0
 * \sa wcfTextGet
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextGetW(wcfClient *wcli,
                                                 const wchar_t *member,
                                                 const wchar_t *field,
                                                 wchar_t *text, int size,
                                                 int *recv_size);

/*!
 * \brief Textが変化した時のイベント
 * \since ver2.0
 * \param wcli
 * \param member member名 (NULLまたは空文字列で自分自身を指す)
 * \param field textの名前
 * \param callback 実行する関数:
 * const char* 型2つ(Memberとfieldの名前が渡される)と void*
 * 型の引数1つを取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第3引数にそのまま渡される。
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfTextChangeEvent(wcfClient *wcli, const char *member, const char *field,
                   wcfEventCallback2 callback, void *user_data);
/*!
 * \brief Textが変化した時のイベント (wstring)
 * \since ver2.0
 * \sa wcfTextChangeEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextChangeEventW(
    wcfClient *wcli, const wchar_t *member, const wchar_t *field,
    wcfEventCallback2W callback, void *user_data);


#ifdef __cplusplus
}
#endif
