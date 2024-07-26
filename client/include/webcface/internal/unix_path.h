#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/config.h"
#endif
#include <spdlog/logger.h>

#if WEBCFACE_EXP_FILESYSTEM
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace std_fs = std::filesystem;
#endif

WEBCFACE_NS_BEGIN
namespace internal {

/*!
 * \brief unix socket のパス
 *
 */
WEBCFACE_DLL std_fs::path WEBCFACE_CALL unixSocketPath(int port);
/*!
 * \brief wslから見たwindows側で開いている unix socket のパス
 *
 */
WEBCFACE_DLL std_fs::path WEBCFACE_CALL unixSocketPathWSLInterop(int port);
/*!
 * \brief wsl1ならtrue
 *
 */
WEBCFACE_DLL bool WEBCFACE_CALL detectWSL1();
/*!
 * \brief wsl2ならtrue
 *
 */
WEBCFACE_DLL bool WEBCFACE_CALL detectWSL2();
/*!
 * \brief wslから見たwindowsのipアドレス
 *
 */
WEBCFACE_DLL std::string WEBCFACE_CALL wsl2Host();

/*!
 * \brief socketファイルがすでにあれば削除する
 *
 */
WEBCFACE_DLL void WEBCFACE_CALL initUnixSocket(
    const std_fs::path &path, const std::shared_ptr<spdlog::logger> &logger);
/*!
 * \brief socketファイルのパーミッション設定
 *
 */
WEBCFACE_DLL void WEBCFACE_CALL updateUnixSocketPerms(
    const std_fs::path &path, const std::shared_ptr<spdlog::logger> &logger);

} // namespace internal
WEBCFACE_NS_END
