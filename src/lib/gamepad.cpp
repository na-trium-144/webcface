#include <webcface/gamepad.hpp>
#include "server.hpp"
namespace WebCFace
{
const GamepadState getGamepad()
{
    for (const auto& cli : clients) {
        std::lock_guard lock(internal_mutex);
        if (cli.second->gamepad_state.connected) {
            return cli.second->gamepad_state;
        }
    }
    return GamepadState();
}
}  // namespace WebCFace