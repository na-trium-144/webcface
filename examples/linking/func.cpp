#include <webcface/webcface.hpp>
#include <iostream>
#include <chrono>
#include <thread>

void test(){
    std::cout << "a" << std::endl;
}
void mainLoop(){
    WebCFace::sendData();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

