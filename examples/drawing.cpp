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

    using namespace WebCFace::Literals;

    int x = 500;
    int y = 500;

    "move.left"_callback = [&] { x -= 10; };
    "move.right"_callback = [&] { x += 10; };
    "move.up"_callback = [&] { y -= 10; };
    "move.down"_callback = [&] { y += 10; };

    while (true) {
        {
            std::lock_guard l(WebCFace::callback_mutex);
            using namespace WebCFace::Layout;
            PageLayout p("field");
            p.clear();
            p << "　　　　　　" << Button("move.up"_callback) << p.endl;
            p << Button("move.left"_callback) << "　　　　　　" << Button("move.right"_callback)
              << p.endl;
            p << "　　　　　　" << Button("move.down"_callback) << p.endl;
            p << "フィールド↓" << p.endl;

            Drawing field(1000, 1000);
            auto field_l = field.createLayer();
            field_l.drawRect(0, 0, 500, 1000, "red");
            field_l.drawRect(500, 0, 1000, 1000, "blue");
            field_l.drawRect(333, 333, 667, 667, "orange").onClick("aca"_callback = [] {
                std::cout << "aca" << std::endl;
            });

            auto robot_l = field.createLayer();
            // robot_l.drawCircle(x, y, 30, "yellow");
            // 直接変数の値を入れることもできるが、
            // ↓のように _value を経由すると値だけ送るので通信量が減る(こともある)
            robot_l.drawCircle("x"_value = x, "y"_value = y, 30, "yellow");

            p << field << p.endl;  // 全部書き終わってからページに入れること

            WebCFace::sendData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
