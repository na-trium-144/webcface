#pragma once
#include <vector>
namespace WebCFace
{
struct GamepadState {
    bool connected = false;
    std::vector<bool> buttons;
    std::vector<double> axes;
};
const GamepadState getGamepad();

}  // namespace WebCFace