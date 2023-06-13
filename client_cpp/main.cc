#include <webcface/webcface.h>
#include <thread>
int main() {
    WebCFace::init("test_client");
    while(true){
        std::this_thread::yield();
    }
}