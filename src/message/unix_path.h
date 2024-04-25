#pragma once
#include <webcface/common/def.h>
#include <spdlog/logger.h>
#include <filesystem>
#include <optional>

WEBCFACE_NS_BEGIN
namespace Message::Path {
// Messageではない気がするがほかにいいnamespaceがない
// (clientでもserverでも使う)

/*!
 * \brief unix socket のパス
 *
 */
WEBCFACE_DLL std::filesystem::path unixSocketPath(int port);
/*!
 * \brief wslから見たwindows側で開いている unix socket のパス
 *
 */
WEBCFACE_DLL std::filesystem::path unixSocketPathWSLInterop(int port);
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
WEBCFACE_DLL void initUnixSocket(const std::filesystem::path &path,
                                 std::shared_ptr<spdlog::logger> logger);
/*!
 * \brief socketファイルのパーミッション設定
 *
 */
WEBCFACE_DLL void updateUnixSocketPerms(const std::filesystem::path &path,
                                        std::shared_ptr<spdlog::logger> logger);

} // namespace Message::Path
WEBCFACE_NS_END
