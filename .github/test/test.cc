#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/logger.h>

int main() {
    webcface::Client wcli{};
    wcli.value("test") = 0;
    wcli.logger()->info("this is info");
    wcli.onMemberEntry().callbackList().append([](webcface::Member m) {});
}
