#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
int main() {
    WebCFace::Client c("example_recv");
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "recv test = " << c.subject("example_main").value("test")
                  << std::endl;
        // c.value("example_main", "test") += 2;
        // -> error: candidate function template not viable: ... method is not
        // marked const
        std::cout << "func2(9, 7, false, \"\") = "
                  << c.subject("example_main")
                         .func("func2")
                         .run(9, 7, false, "")
                         .get()
                  << std::endl;
        c.send();
    }
}