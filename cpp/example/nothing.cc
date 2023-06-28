#include <webcface/webcface.h>
#include <thread>
#include <chrono>
int main() {
    WebCFace::Client c("example_nothing");
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.send();
    }
}