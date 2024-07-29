#pragma once
#include <array>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief クライアントのバージョン
 * \since ver1.2
 *
 * WEBCFACE_VERSION_MAJOR マクロなどでも取得できるが、
 * マクロはコンパイル時に使用したヘッダーのバージョンを表すのに対し
 * こちらはリンク時のバージョンが得られる
 *
 * \return major, minor, rev のバージョン番号
 */
WEBCFACE_DLL extern const std::array<int, 3> version;
/*!
 * \brief クライアントのバージョン(文字列)
 * \since ver1.2
 * \return "x.y.z" 形式の文字列
 */
WEBCFACE_DLL extern const char *version_s;

WEBCFACE_NS_END
