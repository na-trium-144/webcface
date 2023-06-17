#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <webcface/webcface.hpp>
// kbhit() : return true if some key is pressed
// https://stackoverflow.com/questions/29335758/using-kbhit-and-getch-on-linux
int main()
{
    WebCFace::initStdLogger();
    WebCFace::startServer(3001);
    int i = 0;
    while (true) {
        {
            std::cout << "This is cout test " << i << std::endl;
            /* std::cerr << "This is cout err " << i << std::endl; */
            i++;
            std::lock_guard l(WebCFace::callback_mutex);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            WebCFace::sendData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}
