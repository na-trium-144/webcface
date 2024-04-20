#pragma once
#include <string>
#include <string_view>
#include "common/def.h"
#include "common/field_base.h"

WEBCFACE_NS_BEGIN
namespace Encoding {

using NameRef = const void *;

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
 * utf8であることを明確にするために戻り値型をu8stringにしている
 *
 */
WEBCFACE_DLL std::u8string initName(std::string_view name);

/*!
 * \brief wstringをutf8のchar配列に変換する
 * \since ver1.11
 *
 * (unixではたぶん必要ないが、#ifで分岐するのがめんどいので作ってしまう)
 *
 * utf8であることを明確にするために戻り値型をu8stringにしている
 *
 */
WEBCFACE_DLL std::u8string initNameW(std::wstring_view name);

std::u8string_view getNameU8(NameRef name_ref) {
    return static_cast<const char8_t *>(name_ref);
}
/*!
 * \brief utf8の文字列をstringに変換する
 * \since ver1.11
 */
WEBCFACE_DLL std::string getName(std::u8string_view name_ref);
/*!
 * \brief utf8のchar配列(null終端)をstringに変換する
 * \since ver1.11
 */
std::string getName(NameRef name_ref) { return getName(getNameU8(name_ref)); }
/*!
 * \brief utf8の文字列をwstringに変換する
 * \since ver1.11
 */
WEBCFACE_DLL std::wstring getNameW(std::u8string_view name_ref);
/*!
 * \brief utf8のchar配列(null終端)をwstringに変換する
 * \since ver1.11
 */
std::wstring getNameW(NameRef name_ref) {
    return getNameW(getNameU8(name_ref));
}

} // namespace Encoding
WEBCFACE_NS_END
