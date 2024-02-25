#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \param text 表示する文字列
 */
WEBCFACE_DLL wcfViewComponent wcfText(const char *text);
WEBCFACE_DLL wcfViewComponent wcfNewLine();
/*!
 * \param text 表示する文字列
 * \param on_click_member クリック時に実行するFuncのmember
 * またはnullptr(自分自身を表す)
 * \param on_click_field クリック時に実行するFuncの名前
 */
WEBCFACE_DLL wcfViewComponent wcfButton(const char *text,
                                        const char *on_click_member,
                                        const char *on_click_field);

/*!
 * \brief Viewを送信する
 * \param wcli Clientポインタ
 * \param field viewの名前
 * \param components 送信するデータの配列へのポインタ
 * \param size 配列の要素数
 * \return wcliが無効ならWCF_BAD_WCLI
 *
 */
WEBCFACE_DLL wcfStatus wcfViewSet(wcfClient *wcli, const char *field,
                                  const wcfViewComponent *components, int size);

/*!
 * \brief Viewを受信する
 * \param wcli Clientポインタ
 * \param member memberの名前
 * \param field viewの名前
 * \param components 受信したデータを格納する配列へのポインタが返る
 * \param recv_size 実際に受信した要素数が返る
 * \return wcliが無効ならWCF_BAD_WCLI,
 * 対象のmemberやfieldが存在しない場合 WCF_NOT_FOUND
 *
 */
WEBCFACE_DLL wcfStatus wcfViewGet(wcfClient *wcli, const char *member,
                                  const char *field,
                                  wcfViewComponent **components,
                                  int *recv_size);

#ifdef __cplusplus
}
#endif