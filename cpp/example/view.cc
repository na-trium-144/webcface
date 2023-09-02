#include <webcface/webcface.h>
#include <thread>
#include <chrono>
#include <iostream>
int main() {
    WebCFace::Client c("example_view");
    std::cout << 1 << std::endl;
    int i = 0;
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            auto v = c.view("a");
            v << "hello world" << std::endl;
            v << i << WebCFace::newLine();
            v << WebCFace::button("a",
                                  [] { std::cout << "hello" << std::endl; });
        }
        i++;
        c.sync();
    }
}