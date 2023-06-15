#include <webcface/webcface.h>
#include <thread>
#include <iostream>
int main() {
    WebCFace::Client c("test_client");
    c.value("test") = 5;
    while(true){
        std::this_thread::yield();
        c.send();
    }
}