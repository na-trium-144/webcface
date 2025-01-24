#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_C_BEGIN

/*!
 * \brief webcfaceが使用するエンコーディングを設定する
 * \since ver2.0
 *
 * * windowsでは、false(0)の場合webcfaceの各種クラスのインタフェースで使われる
 * char* をすべてANSIエンコーディングとみなし、
 * 内部でutf8と相互変換する。
 * * デフォルトは true(1) (以前のバージョンとの互換性のため)
 * * unixでは効果がない(この設定に関わらず文字列はすべてutf8とみなされ相互変換は行われない)
 * * wchar_t* 型の文字列には影響しない
 * 
 */
WEBCFACE_DLL void WEBCFACE_CALL wcfUsingUTF8(int flag);

/*!
 * \since ver3.0
 * \sa wcfUsingUTF8
 */
WEBCFACE_DLL int WEBCFACE_CALL wcfGetUsingUTF8();

WEBCFACE_C_END
