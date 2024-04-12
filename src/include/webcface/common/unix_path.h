#include "def.h"
#include <filesystem>

WEBCFACE_NS_BEGIN
inline namespace Common {
inline std::filesystem::path unixSocketPath(int port) {
  return "/tmp/webcface/" + std::to_string(port) + ".sock";
}
} // namespace Common
WEBCFACE_NS_END
