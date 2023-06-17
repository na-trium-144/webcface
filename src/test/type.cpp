#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>

#include <webcface/type.hpp>
#include <string>
#include <array>
#include <vector>
#include <type_traits>
using namespace WebCFace;

struct SomeStruct{};

DROGON_TEST(type_check)
{
    CHECK(ValueType::of<int>().repr == ValueType::Repr::int_);
    CHECK(ValueType::of<char>().repr == ValueType::Repr::int_);
    CHECK(ValueType::of<long>().repr == ValueType::Repr::int_);
    CHECK(ValueType::of<bool>().repr == ValueType::Repr::bool_);
    CHECK(ValueType::of<float>().repr == ValueType::Repr::float_);
    CHECK(ValueType::of<double>().repr == ValueType::Repr::float_);
    CHECK(ValueType::of<std::string>().repr == ValueType::Repr::string_);
    CHECK(ValueType::of<const char*>().repr == ValueType::Repr::string_);
    CHECK(ValueType::of<char[5]>().repr == ValueType::Repr::string_);
    CHECK((ValueType::of<std::array<int, 3>>()).repr == ValueType::Repr::vector_);
    CHECK((ValueType::of<std::array<int, 3>>()).child->repr == ValueType::Repr::int_);
    CHECK(ValueType::of<std::vector<int>>().repr == ValueType::Repr::vector_);
    CHECK(ValueType::of<std::vector<int>>().child->repr == ValueType::Repr::int_);
    CHECK((ValueType::of<std::array<std::vector<int>, 3>>()).repr == ValueType::Repr::vector_);
    CHECK((ValueType::of<std::array<std::vector<int>, 3>>()).child->repr == ValueType::Repr::vector_);
    CHECK((ValueType::of<std::array<std::vector<int>, 3>>()).child->child->repr == ValueType::Repr::int_);
    CHECK((ValueType::of<std::vector<std::array<int, 3>>>()).repr == ValueType::Repr::vector_);
    CHECK((ValueType::of<std::vector<std::array<int, 3>>>()).child->repr == ValueType::Repr::vector_);
    CHECK((ValueType::of<std::vector<std::array<int, 3>>>()).child->child->repr == ValueType::Repr::int_);
    CHECK((ValueType::of<SomeStruct>()).repr == ValueType::Repr::unknown_);

}
int main(int argc, char** argv)
{
    return drogon::test::run(argc, argv);
}
