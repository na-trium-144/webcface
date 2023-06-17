#include <chrono>
#include <iostream>
#include <thread>
#include <webcface/webcface.hpp>

int main()
{
    WebCFace::initStdLogger();
    int t = 0;
    WebCFace::startServer(3001);
    WebCFace::setServerName("related1");
    WebCFace::addRelatedServer(3002);
    WebCFace::addFunctionToRobot("shell1", []() { std::cout << "related1のシェル1" << std::endl; });
    WebCFace::addValueFromRobot("value1", 1);

    {
        using namespace WebCFace::Layout;
        using namespace WebCFace::Literals;
        // clang-format off
        WebCFace::addPageLayout("test", {{
            {{ Button("shell1", "shell1"_callback), Button("related2:shell1", "related2:shell1"_callback)}},
            {{ "value1 = ", "value1"_value, "related2:value2 = ", "related2:value2"_value}},
        }});
        // clang-format on
    }

    while (true) {
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
