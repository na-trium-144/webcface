#include "def_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief memberが公開しているValueのリストを得る
 * \since ver2.0
 *
 * * sizeに指定したサイズよりfieldの数が多い場合、
 * size分のfield名を格納する。
 * * sizeに指定したサイズよりfieldの数が少ない場合、
 * size分より後ろの余った部分はそのまま
 *
 * \param wcli
 * \param member member名 (NULLまたは空文字列で自分自身を指す)
 * \param list field名を格納するchar*の配列
 * (size=0ならNULLも可)
 * \param size listの要素数
 * \param field_num 実際のfield数
 * \return wcliが無効ならwcfBadClient
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueEntryList(wcfClient *wcli,
                                                       const char *member,
                                                       const char **list,
                                                       int size,
                                                       int *field_num);
/*!
 * \brief memberが公開しているValueのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfValueEntryListW(wcfClient *wcli,
                                                        const wchar_t *member,
                                                        const wchar_t **list,
                                                        int size,
                                                        int *field_num);
/*!
 * \brief memberが公開しているTextのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextEntryList(wcfClient *wcli,
                                                      const char *member,
                                                      const char **list,
                                                      int size, int *field_num);
/*!
 * \brief memberが公開しているTextのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfTextEntryListW(wcfClient *wcli,
                                                       const wchar_t *member,
                                                       const wchar_t **list,
                                                       int size,
                                                       int *field_num);
/*!
 * \brief memberが公開しているFuncのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncEntryList(wcfClient *wcli,
                                                      const char *member,
                                                      const char **list,
                                                      int size, int *field_num);
/*!
 * \brief memberが公開しているFuncのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfFuncEntryListW(wcfClient *wcli,
                                                       const wchar_t *member,
                                                       const wchar_t **list,
                                                       int size,
                                                       int *field_num);
/*!
 * \brief memberが公開しているViewのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfViewEntryList(wcfClient *wcli,
                                                      const char *member,
                                                      const char **list,
                                                      int size, int *field_num);
/*!
 * \brief memberが公開しているViewのリストを得る
 * \since ver2.0
 * \sa wcfValueEntryEventList
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfViewEntryListW(wcfClient *wcli,
                                                       const wchar_t *member,
                                                       const wchar_t **list,
                                                       int size,
                                                       int *field_num);

/*!
 * \brief Valueが追加された時のイベント
 * \since ver2.0
 * \param wcli
 * \param member member名 (NULLまたは空文字列で自分自身を指す)
 * \param callback 実行する関数:
 * const char* 型2つ(追加されたMemberとfieldの名前が渡される)と void*
 * 型の引数1つを取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第3引数にそのまま渡される。
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfValueEntryEvent(wcfClient *wcli, const char *member,
                   wcfEventCallback2 callback, void *user_data);
/*!
 * \brief Valueが追加された時のイベント (wstring)
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfValueEntryEventW(wcfClient *wcli, const wchar_t *member,
                    wcfEventCallback2W callback, void *user_data);
/*!
 * \brief Textが追加された時のイベント
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfTextEntryEvent(wcfClient *wcli, const char *member,
                  wcfEventCallback2 callback, void *user_data);
/*!
 * \brief Textが追加された時のイベント (wstring)
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfTextEntryEventW(wcfClient *wcli, const wchar_t *member,
                   wcfEventCallback2W callback, void *user_data);
/*!
 * \brief Funcが追加された時のイベント
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncEntryEvent(wcfClient *wcli, const char *member,
                  wcfEventCallback2 callback, void *user_data);
/*!
 * \brief Funcが追加された時のイベント (wstring)
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfFuncEntryEventW(wcfClient *wcli, const wchar_t *member,
                   wcfEventCallback2W callback, void *user_data);
/*!
 * \brief Viewが追加された時のイベント
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfViewEntryEvent(wcfClient *wcli, const char *member,
                  wcfEventCallback2 callback, void *user_data);
/*!
 * \brief Viewが追加された時のイベント (wstring)
 * \since ver2.0
 * \sa wcfValueEntryEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfViewEntryEventW(wcfClient *wcli, const wchar_t *member,
                   wcfEventCallback2W callback, void *user_data);

/*!
 * \brief Memberがsyncした時のイベント
 * \since ver2.0
 * \param wcli
 * \param callback 実行する関数:
 * const char* 型(Memberの名前が渡される)と void*
 * 型の引数を1つずつ取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第2引数にそのまま渡される。
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfMemberSyncEvent(wcfClient *wcli, const char *member,
                   wcfEventCallback1 callback, void *user_data);
/*!
 * \brief Memberがsyncした時のイベント (wstring)
 * \since ver2.0
 * \sa wcfMemberSyncEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfMemberSyncEventW(wcfClient *wcli, const wchar_t *member,
                    wcfEventCallback1W callback, void *user_data);

/*!
 * \brief 最後のsync()の時刻を返す
 * \since ver2.0
 * \return 1970/1/1 0:00(utc) からの経過ミリ秒数で表し、閏秒はカウントしない
 */
WEBCFACE_DLL unsigned long long WEBCFACE_CALL
wcfMemberSyncTime(wcfClient *wcli, const char *member);
/*!
 * \brief 最後のsync()の時刻を返す
 * \since ver2.0
 * \sa wcfMemberSyncTime
 */
WEBCFACE_DLL unsigned long long WEBCFACE_CALL
wcfMemberSyncTimeW(wcfClient *wcli, const wchar_t *member);

/*!
 * \brief Memberが使っているWebCFaceライブラリの識別情報を返す
 * \since ver2.0
 * \todo wstringバージョンも用意したほうがいいかも
 */
WEBCFACE_DLL const char *WEBCFACE_CALL wcfMemberLibName(wcfClient *wcli,
                                                        const char *member);
/*!
 * \brief Memberが使っているWebCFaceのバージョンを返す
 * \since ver2.0
 */
WEBCFACE_DLL const char *WEBCFACE_CALL wcfMemberLibVersion(wcfClient *wcli,
                                                           const char *member);
/*!
 * \brief MemberのIPアドレスを返す
 * \since ver2.0
 */
WEBCFACE_DLL const char *WEBCFACE_CALL wcfRemoteAddr(wcfClient *wcli,
                                                     const char *member);

/*!
 * \brief memberの通信速度を取得
 * \since ver2.0
 *
 * * 初回の呼び出しで通信速度データをリクエストし、
 * wcfSync()後通信速度が得られるようになる
 * * Client自身に対しても使用可能
 *
 * \param wcli
 * \param member
 * \param value pingの往復時間(ms)が返る
 * \return wcliが無効ならwcfBadClient,
 * また値を受信していなければwcfNotReceived
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfMemberPingStatus(wcfClient *wcli,
                                                         const char *member,
                                                         int *value);
/*!
 * \brief memberの通信速度を取得
 * \since ver2.0
 * \sa wcfMemberPingStatus
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL wcfMemberPingStatusW(wcfClient *wcli,
                                                          const wchar_t *member,
                                                          int *value);
/*!
 * \brief Memberの通信速度が更新された時のイベント
 * \since ver2.0
 * \param wcli
 * \param callback 実行する関数:
 * const char* 型(Memberの名前が渡される)と void*
 * 型の引数を1つずつ取り、何もreturnしない。
 * \param user_data 関数に引数として渡す追加のデータ
 * callbackが呼び出されるときに第2引数にそのまま渡される。
 * \return wcliが無効ならwcfBadClient
 *
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfMemberPingEvent(wcfClient *wcli, const char *member,
                   wcfEventCallback1 callback, void *user_data);
/*!
 * \brief Memberがsyncした時のイベント (wstring)
 * \since ver2.0
 * \sa wcfMemberPingEvent
 */
WEBCFACE_DLL wcfStatus WEBCFACE_CALL
wcfMemberPingEventW(wcfClient *wcli, const wchar_t *member,
                    wcfEventCallback1W callback, void *user_data);


#ifdef __cplusplus
}
#endif
