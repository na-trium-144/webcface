#pragma once
#include <webcface/common/def.h>
#include <spdlog/logger.h>
#include <filesystem>
#include <optional>

WEBCFACE_NS_BEGIN
namespace Message::Path {
// Messageではない気がするがほかにいいnamespaceがない
// (clientでもserverでも使う)

WEBCFACE_DLL std::filesystem::path unixSocketPath(int port);
WEBCFACE_DLL std::optional<std::filesystem::path>
unixSocketPathWSLInterop(int port);

WEBCFACE_DLL void initUnixSocket(const std::filesystem::path &path,
                                 std::shared_ptr<spdlog::logger> logger);
WEBCFACE_DLL void updateUnixSocketPerms(const std::filesystem::path &path,
                                        std::shared_ptr<spdlog::logger> logger);

} // namespace Message
WEBCFACE_NS_END
