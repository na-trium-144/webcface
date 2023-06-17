#include <chrono>
#include <iostream>
#include <thread>
#include <webcface/webcface.hpp>

int main()
{
    WebCFace::initStdLogger();
    int t = 0;
    WebCFace::startServer(3002);
    WebCFace::setServerName("related2");
    WebCFace::addRelatedServer(3001);
    WebCFace::addFunctionToRobot("shell1", []() { std::cout << "related2のシェル1" << std::endl; });
    WebCFace::addValueFromRobot("value2", 2);

    {
        using namespace WebCFace::Layout;
        using namespace WebCFace::Literals;
        Drawing test_drawing(100, 100);
        test_drawing.addLayer("hoge:pien");
        test_drawing.addLayer("related1:aaa");
        auto layer2 = test_drawing.createLayer("bbb");
        layer2.drawRect(30, 30, 70, 70, "orange").onClick("shell1"_callback);
        // clang-format off
        WebCFace::addPageLayout("test", {{
            {{ Button("shell1"_callback), Button("related1:shell1"_callback)}},
            {{ "related1:value1 = ", "related1:value1"_value, "value2 = ", "value2"_value}},
            test_drawing, 
        }});
        // clang-format on
    }

    while (true) {
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
