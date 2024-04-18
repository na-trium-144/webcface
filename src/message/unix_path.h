#pragma once
#include <webcface/common/def.h>
#include <filesystem>
#include <optional>

WEBCFACE_NS_BEGIN
namespace Message {
// Messageではない気がするがほかにいいnamespaceがない
// (clientでもserverでも使う)

WEBCFACE_DLL std::filesystem::path unixSocketPath(int port);
WEBCFACE_DLL std::optional<std::filesystem::path>
unixSocketPathWSLInterop(int port);

} // namespace Message
WEBCFACE_NS_END
