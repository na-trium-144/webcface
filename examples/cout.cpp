#include <iostream>
#include <webcface/webcface.hpp>

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

    auto t1 = ptimer::ptimer([] {
        for (int i = 0; i < 10000; i++) {
            std::cout << "hello, world!" << std::endl;
        }
    });


    WebCFace::initStdLogger();
    WebCFace::startServer(3001);

    auto t2 = ptimer::ptimer([] {
        for (int i = 0; i < 10000; i++) {
            std::cout << "hello, world!" << std::endl;
        }
    });

    std::cerr << t1 << std::endl;
    std::cerr << t2 << std::endl;

}
