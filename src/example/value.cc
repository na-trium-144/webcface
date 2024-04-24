#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <thread>
#include <chrono>

struct A {
    int x = 1, y = 2;
    A() = default;
    // Dict → A に変換
    A(const webcface::Value::Dict &d) : x(d["x"]), y(d["y"]) {}
    // A → Dictに変換
    operator webcface::Value::Dict() const {
        return {{"x", x}, {"y", y}, {"nest", {{"a", 3}, {"b", 4}}}};
    }
};
int main() {
    webcface::Client wcli("example_value");

    // wcli.value("test").set(0);
    wcli.value("test") = 0;

    // structをDictに変換するとまとめて送信することができる
    wcli.value("dict") = A();

    webcface::Field field = wcli.child("sub_field");
    field.value("a") = 1; // wcli.value("sub_field.a")
    field.child("b").value() = 2; // wcli.value("sub_field.b")
    field["c"].value() = 3; // wcli.value("sub_field.c")

    // 文字列送信
    wcli.text("str") = "hello";

    double i = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // valueを更新
        wcli.value("test") = i;
        wcli.value("not_frequent") = static_cast<int>(i / 10);

        i += 0.5;

        wcli.sync();
    }
}
