#pragma once
#include <string>
#include <string_view>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Encoding {
/*!
 * \brief webcfaceが使用するエンコーディングを設定する
 * \since ver1.12
 *
 * * windowsでは、falseの場合webcfaceの各種クラスのインタフェースで使われる
 * std::string をすべてANSIエンコーディングとみなし、
 * 内部でutf8と相互変換する。
 * * デフォルトは true (以前のバージョンとの互換性のため)
 * * unixでは効果がない。
 * * std::wstring をspdlogに渡して使用する場合、内部で
 * wstring→utf-8 の変換がされる場合があるのでtrueにすることを推奨
 * * spdlogのloggerではなく Client::loggerWStreamBuf() を使用する場合は、
 * 出力するコンソールのコードページに合わせた設定にする必要がある
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
 * windowsではusingUTF8(false)の場合ANSIからutf8へエンコーディングの変換を行うが、
 * usingUTF8(true)の場合なにもせずそのままコピーする。
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
 *
 * windowsでusingUTF8(false)の場合はANSIに、
 * それ以外の場合なにもせずそのままコピーする。
 *
 */
WEBCFACE_DLL std::string decode(std::u8string_view name_ref);
/*!
 * \brief utf8の文字列をwstringに変換する
 * \since ver1.12
 */
WEBCFACE_DLL std::wstring decodeW(std::u8string_view name_ref);

/*!
 * \brief stringをwstringに変換する
 * \since ver1.12
 */
WEBCFACE_DLL std::wstring toWide(std::string_view name_ref);
/*!
 * \brief wstringをstringに変換する
 * \since ver1.12
 */
WEBCFACE_DLL std::string toNarrow(std::wstring_view name_ref);

/*!
 * \brief stringをu8stringにキャストする
 * \since ver1.12
 */
WEBCFACE_DLL std::u8string_view castToU8(std::string_view name);
/*!
 * \since ver1.12
 */
inline std::u8string_view castToU8(const char *data, std::size_t size) {
    return castToU8(std::string_view(data, size));
}

/*!
 * \brief u8stringをstringにキャストする
 * \since ver1.12
 */
WEBCFACE_DLL std::string_view castFromU8(std::u8string_view name);

} // namespace Encoding
WEBCFACE_NS_END
