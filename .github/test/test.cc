#include <webcface/webcface.h>

int main() {
    webcface::Client wcli{};
    wcli.value("test") = 0;
    wcli.log().append(webcface::level::info, "this is info");
    wcli.onMemberEntry([](webcface::Member m) {});
    int i;
    wcli.func("func").set([&](int a) { i += a; });

    webcface::server::Server s(7530, webcface::level::info);
}
