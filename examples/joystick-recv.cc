#include <cassert>
#include <iostream>
#include <webcface/client.h>
#include <webcface/value.h>

int main() {
    // webcface::logger_internal_level = spdlog::level::trace;
    webcface::Client c;
    c.waitConnection();
    std::vector<webcface::Member> joysticks;
    for (const auto &m : c.members()) {
        if (m.name().substr(0, 17) == "webcface-joystick" ||
            m.name().substr(0, 14) == "webui-joystick") {
            assert(m.exists());
            if (m.connected()) {
                std::cout << "member '" << m.name() << "': connected"
                          << std::endl;
                joysticks.push_back(m);
            } else {
                std::cout << "member '" << m.name()
                          << "': exists but disconnected" << std::endl;
            }
        }
    }
    while (true) {
        for (const auto &m : joysticks) {
            if (m.child("game_buttons").hasChildren()) {
                std::cout << "game_buttons: ";
                for (const auto &v : m.child("game_buttons").children()) {
                    std::cout << v.lastName() << "=" << v.value().get() << ", ";
                }
                std::cout << std::endl;
                std::cout << "game_axes: ";
                for (const auto &v : m.child("game_axes").children()) {
                    std::cout << v.lastName() << "=" << v.value().get() << ", ";
                }
                std::cout << std::endl;
            } else {
                std::cout << "buttons: ";
                for (const auto &v : m.child("buttons").value().getVec()) {
                    std::cout << v << ", ";
                }
                std::cout << std::endl;
                std::cout << "axes: ";
                for (const auto &v : m.child("axes").value().getVec()) {
                    std::cout << v << ", ";
                }
                std::cout << std::endl;
                std::cout << "hats: ";
                for (const auto &v : m.child("hats").value().getVec()) {
                    std::cout << v << ", ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
        c.loopSyncFor(std::chrono::milliseconds(100));
    }
}