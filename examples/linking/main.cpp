#include <webcface/webcface.hpp>
void test();
void mainLoop();

int main(){
    WebCFace::startServer();
    WebCFace::addFunctionToRobot("test", test);
    while(true){
        mainLoop();
    }
}
