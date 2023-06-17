#include <chrono>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <webcface/webcface.hpp>
#include <iostream>

// https://github.com/na-trium-144/processing-timer/blob/main/ptimer.hpp
#include <ctime>
#include <functional>
namespace ptimer {
long ptimer(std::function<void()> func) {
  timespec start_t, end_t;
  clock_gettime(CLOCK_MONOTONIC, &start_t);
  func();
  clock_gettime(CLOCK_MONOTONIC, &end_t);
  return (end_t.tv_sec - start_t.tv_sec) * 1e9L + (end_t.tv_nsec - start_t.tv_nsec);
}
} // namespace ptimer


int main()
{
    WebCFace::initStdLogger();
    WebCFace::startServer(3001);
    int t = 0;
    WebCFace::addSharedVarFromRobot("t1", {}, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t,
        t, t, t, t, t, t, t, t, t, t, t, t);
    WebCFace::addSharedVarFromRobot("t2", {}, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t,
        t, t, t, t, t, t, t, t, t, t, t, t);
    WebCFace::addSharedVarFromRobot("t3", {}, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t,
        t, t, t, t, t, t, t, t, t, t, t, t);

    while (true) {
        t++;
        std::cout << ptimer::ptimer([]{
            WebCFace::sendData();
        }) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
