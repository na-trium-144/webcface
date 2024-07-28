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
std_fs::path unixSocketPath(int port);
/*!
 * \brief wslから見たwindows側で開いている unix socket のパス
 *
 */
std_fs::path unixSocketPathWSLInterop(int port);
/*!
 * \brief wsl1ならtrue
 *
 */
bool detectWSL1();
/*!
 * \brief wsl2ならtrue
 *
 */
bool detectWSL2();
/*!
 * \brief wslから見たwindowsのipアドレス
 *
 */
std::string wsl2Host();

/*!
 * \brief socketファイルがすでにあれば削除する
 *
 */
void initUnixSocket(const std_fs::path &path,
                    const std::shared_ptr<spdlog::logger> &logger);
/*!
 * \brief socketファイルのパーミッション設定
 *
 */
void updateUnixSocketPerms(const std_fs::path &path,
                           const std::shared_ptr<spdlog::logger> &logger);

} // namespace internal
WEBCFACE_NS_END
