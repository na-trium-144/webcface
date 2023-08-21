#include <webcface/webcface.h>
#include <thread>
#include <chrono>
int main() {
    WebCFace::Client c("example_nothing");
    c.loggerDebug() << "this is debug" << std::endl;
    c.loggerInfo() << "this is info" << std::endl;
    c.loggerWarning() << "this is warning\nwarning" << std::endl;
    c.loggerError() << "this is error" << std::endl;
    c.loggerCritical() << "this is critical" << std::endl;
    c.logger(7) << "this is more important than above" << std::endl;
    c.logger(-3) << "this is less important than above" << std::endl;
    while(true){
        std::this_thread::yield();
    }
}