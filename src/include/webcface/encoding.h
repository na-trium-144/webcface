#pragma once
#include <string>
#include <string_view>
#include <webcface/common/def.h>
#include "common/field_base.h"

WEBCFACE_NS_BEGIN
namespace Encoding {

/*!
 * \brief webcfaceが使用するエンコーディングを設定する
 * \since ver1.12
 *
 * * windowsでは、falseの場合webcfaceの各種クラスのインタフェースで使われる
 * std::string をすべてACPエンコーディングとみなし、
 * 内部でutf8と相互変換する。
 * * デフォルトは true (以前のバージョンとの互換性のため)
 * * unixでは効果がない。
 * * windows,unixともに std::wstring には影響がない
 * * サーバーではこの設定に関わらず常にutf8を使用する。
 *
 */
WEBCFACE_DLL void usingUTF8(bool flag);
/*!
 * \brief webcfaceが使用するエンコーディングを取得する
 * \since ver1.12
 *
 */
WEBCFACE_DLL bool usingUTF8();

/*!
 * \brief stringをutf8のchar配列に変換する
 * \since ver1.12
 *
 * windowsではエンコーディングの変換を行うが、
 * unixではなにもせずそのままコピーする。
 *
 * utf8であることを明確にするために戻り値型をu8stringにしている
 *
 */
WEBCFACE_DLL std::u8string encode(std::string_view name);

/*!
 * \brief wstringをutf8のchar配列に変換する
 * \since ver1.12
 *
 * (unixではたぶん必要ないが、#ifで分岐するのがめんどいので作ってしまう)
 *
 * utf8であることを明確にするために戻り値型をu8stringにしている
 *
 */
WEBCFACE_DLL std::u8string encodeW(std::wstring_view name);

/*!
 * \brief utf8の文字列をstringに変換する
 * \since ver1.12
 */
WEBCFACE_DLL std::string decode(std::u8string_view name_ref);
/*!
 * \brief utf8の文字列をwstringに変換する
 * \since ver1.12
 */
WEBCFACE_DLL std::wstring decodeW(std::u8string_view name_ref);

} // namespace Encoding
WEBCFACE_NS_END
