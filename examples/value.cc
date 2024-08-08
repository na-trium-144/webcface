#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <thread>
#include <chrono>

int main() {
    webcface::Client wcli("example_value");

    // wcli.value("test").set(0);
    wcli.value("test") = 0;

    webcface::Field field = wcli.child("sub_field");
    field.value("a") = 1;         // wcli.value("sub_field.a")
    field.child("b").value() = 2; // wcli.value("sub_field.b")
    field["c"].value() = 3;       // wcli.value("sub_field.c")

    // wcli.value("vec") = {1, 2, 3};
    webcface::Value vec = wcli.value("vec");
    for (int i = 0; i < 3; i++) {
        vec.push_back(i);
    }

    // 文字列送信
    wcli.text("str") = "hello";

    double i = 0;

    while (true) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // valueを更新
        wcli.value("test") = i;
        wcli.value("not_frequent") = static_cast<int>(i / 10);

        i += 0.5;

        wcli.waitSyncFor(std::chrono::milliseconds(100));
    }
}
