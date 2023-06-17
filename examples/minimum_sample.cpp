#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <webcface/webcface.hpp>

void hoge(int a)
{
    std::cout << a << std::endl;
}
int main()
{
    WebCFace::initStdLogger();
    int t = 0;
    WebCFace::startServer(3001);
    WebCFace::addFunctionToRobot("shell1", []() { std::cout << "シェル1" << std::endl; }, {});
    WebCFace::addFunctionToRobot("hoge", hoge);

    while (true) {
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}
