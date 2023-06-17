#pragma once
#include <vector>
#include <string>
namespace WebCFace
{
struct GamepadState {
    bool connected = false;
    std::vector<bool> buttons;
    std::vector<double> axes;
};
const GamepadState getGamepad();

void setButtonName(const std::vector<std::string>& name);
void setAxisName(const std::vector<std::string>& name);

}  // namespace WebCFace