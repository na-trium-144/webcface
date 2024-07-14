#pragma once
#include <webcface/common/def.h>
#include <spdlog/logger.h>

#ifdef WEBCFACE_EXP_FILESYSTEM
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace std_fs = std::filesystem;
#endif

WEBCFACE_NS_BEGIN
namespace message::Path {
// Messageではない気がするがほかにいいnamespaceがない
// (clientでもserverでも使う)

/*!
 * \brief unix socket のパス
 *
 */
WEBCFACE_DLL std_fs::path unixSocketPath(int port);
/*!
 * \brief wslから見たwindows側で開いている unix socket のパス
 *
 */
WEBCFACE_DLL std_fs::path unixSocketPathWSLInterop(int port);
/*!
 * \brief wsl1ならtrue
 *
 */
WEBCFACE_DLL bool detectWSL1();
/*!
 * \brief wsl2ならtrue
 *
 */
WEBCFACE_DLL bool detectWSL2();
/*!
 * \brief wslから見たwindowsのipアドレス
 *
 */
WEBCFACE_DLL std::string wsl2Host();

/*!
 * \brief socketファイルがすでにあれば削除する
 *
 */
WEBCFACE_DLL void initUnixSocket(const std_fs::path &path,
                                 const std::shared_ptr<spdlog::logger> &logger);
/*!
 * \brief socketファイルのパーミッション設定
 *
 */
WEBCFACE_DLL void
updateUnixSocketPerms(const std_fs::path &path,
                      const std::shared_ptr<spdlog::logger> &logger);

} // namespace message::Path
WEBCFACE_NS_END
