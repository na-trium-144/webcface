#pragma once
#include <string>
#include <vector>
#include "common/def.h"
#include "common/field_base.h"

WEBCFACE_NS_BEGIN
namespace Encoding {
/*!
 * \brief webcfaceが使用するエンコーディングを設定する
 * \since ver1.11
 *
 * * windowsでは、falseの場合webcfaceの各種クラスのインタフェースで使われる
 * std::string をすべてACPエンコーディングとみなし、
 * 内部でutf8と相互変換する。
 * * デフォルトは true (ver1.10以前との互換性のため)
 * * unixでは効果がない。
 * * windows,unixともに std::wstring には影響がない
 * * サーバーではこの設定に関わらず常にutf8を使用する。
 *
 */
WEBCFACE_DLL void usingUTF8(bool flag);
/*!
 * \brief webcfaceが使用するエンコーディングを取得する
 * \since ver1.11
 *
 */
WEBCFACE_DLL bool usingUTF8();

/*!
 * \brief stringをutf8のchar配列に変換する
 * \since ver1.11
 *
 * windowsではエンコーディングの変換を行うが、
 * unixではなにもせずそのままコピーする。
 *
 * 必ずnull終端のデータとして返る
 *
 */
WEBCFACE_DLL std::vector<char> initName(const std::string &name);

/*!
 * \brief wstringをutf8のchar配列に変換する
 * \since ver1.11
 *
 * (unixではたぶん必要ないが、#ifで分岐するのがめんどいので作ってしまう)
 *
 * 必ずnull終端のデータとして返る
 *
 */
WEBCFACE_DLL std::vector<char> initNameW(const std::wstring &name);

/*!
 * \brief utf8のchar配列をstringに変換する
 * \since ver1.11
 *
 */
WEBCFACE_DLL std::string getName(const void *name_ref);
/*!
 * \brief utf8のchar配列をwstringに変換する
 * \since ver1.11
 *
 */
WEBCFACE_DLL std::wstring getNameW(const void *name_ref);

/*!
 * \brief wstringをstringに変換する
 * \since ver1.11
 *
 */
inline std::string toUTF8(const std::wstring &wstr) {
    return getName(initNameW(wstr).data());
}

} // namespace Encoding
WEBCFACE_NS_END
