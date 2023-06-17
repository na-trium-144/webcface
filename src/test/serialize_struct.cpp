#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>

#include <webcface/serialize.hpp>
#include <webcface/deserialize.hpp>
#include <string>
#include <vector>
#include <array>

using namespace WebCFace;

DROGON_TEST(serialize_struct)
{
    const std::string str = "teststring";
    const bool hoge = false;
    const int integer = 12345;
    const std::vector<int> vec = {6, 7, 8, 9, 10};
    const std::array<int, 5> arr = {6, 7, 8, 9, 10};
    auto cb = serialize_multi({"str", "boolean", "integer", "vec", "arr"}, str, hoge, integer, vec, arr);
    CHECK(deserialize<std::string>(cb["str"]) == str);
    CHECK(deserialize<bool>(cb["boolean"]) == hoge);
    CHECK(deserialize<int>(cb["integer"]) == integer);

    auto vec2 = deserialize<std::vector<int>>(cb["vec"]);
    for (int i = 0; i < static_cast<int>(vec2.size()); i++) {
        CHECK(vec2[i] == vec[i]);
    }
    auto arr2 = deserialize<std::array<int, 5>>(cb["arr"]);
    for (int i = 0; i < static_cast<int>(arr2.size()); i++) {
        CHECK(arr2[i] == arr[i]);
    }
}
int main(int argc, char** argv)
{
    return drogon::test::run(argc, argv);
}
