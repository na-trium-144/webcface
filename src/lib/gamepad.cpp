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
void setButtonName(const std::vector<std::string>& name){
    std::lock_guard lock(internal_mutex);
    button_name = name;
    setting_changed = true;
}
void setAxisName(const std::vector<std::string>& name){
    std::lock_guard lock(internal_mutex);
    axis_name = name;
    setting_changed = true;
}

}  // namespace WebCFace