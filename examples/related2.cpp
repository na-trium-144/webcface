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
        // clang-format off
        WebCFace::addPageLayout("test", {{
            {{ Button("shell1"_callback), Button("related1:shell1"_callback)}},
            {{ "related1:value1 = ", "related1:value1"_value, "value2 = ", "value2"_value}},
        }});
        // clang-format on
    }

    while (true) {
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
