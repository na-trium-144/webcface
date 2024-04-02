#pragma once
#include <array>
#include "def.h"

WEBCFACE_NS_BEGIN
inline namespace Version {
//! (ver1.2で追加) クライアントのバージョン
/*!
 * WEBCFACE_VERSION_MAJOR マクロなどでも取得できるが、
 * マクロはコンパイル時に使用したヘッダーのバージョンを表すのに対し
 * こちらはリンク時のバージョンが得られる
 *
 * \return major, minor, rev のバージョン番号
 */
WEBCFACE_DLL extern const std::array<int, 3> version;
//! (ver1.2で追加) クライアントのバージョン(文字列)
/*!
 * \return "x.y.z" 形式の文字列
 */
WEBCFACE_DLL extern const char *version_s;
} // namespace Version
WEBCFACE_NS_END
