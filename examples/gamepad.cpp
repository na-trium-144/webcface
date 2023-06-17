#include <chrono>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <webcface/webcface.hpp>

int main()
{
    WebCFace::startServer(3001);
    WebCFace::setButtonName({"○", "×", "△", "□", "ほげ", "ふが"});
    WebCFace::setAxisName({"Lよこ", "Lたて", "Rよこ", "Rたて"});

    while (true) {
        auto gamepad = WebCFace::getGamepad();
        if (gamepad.connected) {
            std::cout << "buttons(" << gamepad.buttons.size() << "): ";
            for (int i = 0; i < gamepad.buttons.size(); i++) {
                if (gamepad.buttons[i]) {
                    std::cout << i << " ";
                }
            }
            std::cout << std::endl;
            std::cout << "axes(" << gamepad.axes.size() << "): ";
            for (int i = 0; i < gamepad.axes.size(); i++) {
                if (gamepad.axes[i]) {
                    std::cout << i << "=" << gamepad.axes[i] << ", ";
                }
            }
            std::cout << std::endl;
        }

        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
