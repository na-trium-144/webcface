#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>

#include <webcface/serialize.hpp>
#include <webcface/deserialize.hpp>
#include <string>
#include <vector>
#include <type_traits>
using namespace WebCFace;


DROGON_TEST(serialize_int)
{
    CHECK(deserialize<int>(serialize(19)) == 19);
    CHECK(deserialize<int>(serialize(-2147483647)) == -2147483647);
}
DROGON_TEST(serialize_string)
{
    CHECK(deserialize<std::string>(serialize("hoge")) == "hoge");
}
DROGON_TEST(serialize_bool)
{
    CHECK(deserialize<bool>(serialize(false)) == false);
    CHECK(deserialize<bool>(serialize(true)) == true);
}

DROGON_TEST(serialize_vec_string)
{
    std::vector<std::string> x;
    for (int i = 0; i < 10; i++)
        x.push_back("hoge" + std::to_string(i));
    auto j = serialize(x);
    for (int i = 0; i < 10; i++) {
        CHECK(deserialize<std::remove_reference_t<decltype(x)>>(j)[i] == x[i]);
    }
}
DROGON_TEST(serialize_vec_int)
{
    std::vector<int> x;
    for (int i = 0; i < 10; i++)
        x.push_back(i);
    auto j = serialize(x);
    for (int i = 0; i < 10; i++) {
        CHECK(deserialize<std::remove_reference_t<decltype(x)>>(j)[i] == x[i]);
    }
}
DROGON_TEST(serialize_array_int)
{
    std::array<int, 3> x{10, 20, 40};
    auto j = serialize(x);
    for (int i = 0; i < 3; i++) {
        CHECK(deserialize<std::remove_reference_t<decltype(x)>>(j)[i] == x[i]);
    }
}
DROGON_TEST(serialize_vec_array_int)
{
    std::vector<std::array<int, 3>> x = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
    };
    auto j = serialize(x);
    for (int i = 0; i < 3; i++) {
        for (int k = 0; i < 3; i++) {
            CHECK(deserialize<std::remove_reference_t<decltype(x)>>(j)[i][k] == x[i][k]);
        }
    }
}
int main(int argc, char** argv)
{
    return drogon::test::run(argc, argv);
}
