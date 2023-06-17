#include <chrono>
#include <thread>
#include <iostream>
#include <webcface/webcface.hpp>

int main()
{
    std::cout << WEBCFACE_STATIC_DIR << std::endl;
    WebCFace::startServer(3001);
    WebCFace::addGeneratedFunctions();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

namespace Functions{
    void test(){
        std::cout << "hello world" << std::endl;
    }
}
