#include <cmath>
#include <webcface/client.h>
#include <webcface/plot.h>
#include <webcface/value.h>

int main(){
    webcface::Client wcli("example_plot");

    while(true){
        auto v1a = wcli.value("test1_value.a");
        auto v1b = wcli.value("test1_value.b");
        static double i1a = 0, i1b = 0;
        v1a.set(std::sin(i1a += 0.1));
        v1b.set(std::sin(i1b += 0.075));
        auto p1 = wcli.plot("test1");
        p1 << webcface::trace1(v1a).color(webcface::ViewColor::red);
        p1 << webcface::trace1(v1b).color(webcface::ViewColor::green);
        p1.sync();

        auto v2x = wcli.value("test2_value.x");
        auto v2y = wcli.value("test2_value.y");
        static double i2x = 0, i2y = 0;
        v2x.set(std::sin(i2x += 0.1));
        v2y.set(std::cos(i2y += 0.075));
        wcli.plot("test2").set({webcface::trace2(v2x, v2y)});

        wcli.loopSyncFor(std::chrono::milliseconds(100));
    }
}