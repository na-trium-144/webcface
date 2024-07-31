#include <webcface/webcface.h>

int main() {
    webcface::Client wcli{};
    wcli.value("test") = 0;
    wcli.log().append(webcface::level::info, "this is info");
    wcli.onMemberEntry([](const webcface::Member &m) {});
    int i;
    wcli.func("func").set([&](int a) { i += a; });
}
