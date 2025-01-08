#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include "webcface/internal/log_history.h"
#include "webcface/field.h"
#include <webcface/member.h>
#include <webcface/value.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class ValueTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
        callback_called = 0;
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
    FieldBase fieldBase(const SharedString &member,
                        std::string_view name) const {
        return FieldBase{member, SharedString::fromU8String(name)};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{SharedString::fromU8String(member),
                         SharedString::fromU8String(name)};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString::fromU8String(name)};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, SharedString::fromU8String(member),
                     SharedString::fromU8String(name)};
    }
    template <typename T1, typename T2>
    Value value(const T1 &member, const T2 &name) {
        return Value{field(member, name)};
    }
    template <std::size_t... Shape, typename T1, typename T2>
    ValueFixed<Shape...> valueFixed(const T1 &member, const T2 &name) {
        return ValueFixed<Shape...>{field(member, name)};
    }
    int callback_called;
    template <typename V>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
    auto callbackVoid() {
        return [&]() { ++callback_called; };
    }
};

TEST_F(ValueTest, field) {
    EXPECT_EQ(value("a", "b").member().name(), "a");
    EXPECT_EQ(value("a", "b").member().nameW(), L"a");
    EXPECT_EQ(value("a", "b").name(), "b");
    EXPECT_EQ(value("a", "b").nameW(), L"b");
    EXPECT_EQ(value("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(value("a", "b").child(L"c").name(), "b.c");
    EXPECT_EQ(value("a", "b.c").parent().name(), "b");
    EXPECT_EQ(valueFixed<1>("a", "b").member().name(), "a");
    EXPECT_EQ(valueFixed<1>("a", "b").member().nameW(), L"a");
    EXPECT_EQ(valueFixed<1>("a", "b").name(), "b");
    EXPECT_EQ(valueFixed<1>("a", "b").nameW(), L"b");
    EXPECT_EQ(valueFixed<1>("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(valueFixed<1>("a", "b").child(L"c").name(), "b.c");
    EXPECT_EQ(valueFixed<1>("a", "b.c").parent().name(), "b");


    EXPECT_THROW(Value().tryGet(), std::runtime_error);
    EXPECT_THROW(ValueFixed<1>().tryGet(), std::runtime_error);
}
TEST_F(ValueTest, eventTarget) {
    value("a", "b").onChange(callback<Value>());
    data_->value_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    value("a", "b").onChange(nullptr);
    EXPECT_FALSE(*data_->value_change_event["a"_ss]["b"_ss]);
    value("a", "b").onChange(callbackVoid());
    data_->value_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;

    valueFixed<1>("a", "c").onChange(callback<ValueFixed<1>>());
    data_->value_change_event["a"_ss]["c"_ss]->operator()(field("a", "c"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    value("a", "c").onChange(nullptr);
    EXPECT_FALSE(*data_->value_change_event["a"_ss]["c"_ss]);
    value("a", "c").onChange(callbackVoid());
    data_->value_change_event["a"_ss]["c"_ss]->operator()(field("a", "c"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ValueTest, valueSet) {
    data_->value_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Value)>>(callback<Value>());
    value(self_name, "b").set(123);
    valueFixed<>(self_name, "b2").set(123);
    valueFixed<1>(self_name, "b3").set(123);
    valueFixed<1, 1, 1>(self_name, "b4").set(123);
    // valueFixed<0>(self_name, "b5").set(123);
    // valueFixed<2>(self_name, "b5").set(123);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "b"_ss)).at(0), 123);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "b2"_ss)).at(0), 123);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "b3"_ss)).at(0), 123);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "b4"_ss)).at(0), 123);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(value("a", "b").set(123), std::invalid_argument);
    EXPECT_THROW(valueFixed<>("a", "b").set(123), std::invalid_argument);
    EXPECT_THROW(valueFixed<1>("a", "b").set(123), std::invalid_argument);
    EXPECT_THROW((valueFixed<1, 1, 1>("a", "b").set(123)),
                 std::invalid_argument);
}
TEST_F(ValueTest, valueSetVec) {
    data_->value_change_event[self_name]["d"_ss] =
        std::make_shared<std::function<void(Value)>>(callback<Value>());
    value(self_name, "d").set({1, 2, 3, 4, 5});
    value(self_name, "d8").set({true, 2, 3, 4, 5.0});
    value(self_name, "d2").set(std::vector<double>{1, 2, 3, 4, 5});
    value(self_name, "d3").set(std::vector<int>{1, 2, 3, 4, 5});
    value(self_name, "d4").set(std::array<int, 5>{1, 2, 3, 4, 5});
    int a[5] = {1, 2, 3, 4, 5};
    value(self_name, "d5").set(a);
    auto d6 = value(self_name, "d6");
    d6.set(1);
    d6.push_back(2);
    d6.push_back(3);
    d6.resize(5);
    d6[3].set(4);
    d6[4] = 5;
    EXPECT_THROW(d6[5].set(6), std::out_of_range);
    EXPECT_THROW(d6[-1].set(0), std::out_of_range);
    value(self_name, "d7").resize(5);

    valueFixed<5>(self_name, "d9").set({1, 2, 3, 4, 5});
    valueFixed<5>(self_name, "d10").set(std::array<int, 5>{1, 2, 3, 4, 5});
    valueFixed<5>(self_name, "d11").set(std::vector<int>{1, 2, 3, 4, 5});
    valueFixed<5>(self_name, "d12")
        .set(std::vector<std::vector<int>>{{1, 2}, {3, 4, 5}});
    valueFixed<5>(self_name, "d13")
        .set(std::array<std::array<int, 1>, 5>{
            {{{1}}, {{2}}, {{3}}, {{4}}, {{5}}}});
    EXPECT_THROW(valueFixed<5>(self_name, "d14").set({1, 2, 3}),
                 std::invalid_argument);
    EXPECT_THROW(valueFixed<5>(self_name, "d14")
                     .set(std::vector<std::vector<int>>{{1, 2}, {3, 4, 5, 6}}),
                 std::invalid_argument);
    auto d15 = valueFixed<5>(self_name, "d15");
    d15[0].set(1);
    d15[4] = 5;
    EXPECT_THROW(d15[5].set(6), std::out_of_range);
    EXPECT_THROW(d15[-1].set(0), std::out_of_range);
    auto d16 = valueFixed<1, 5, 1>(self_name, "d16");
    d16[0][0][0].set(1);
    d16[1][-1][0] = 5;
    EXPECT_THROW(d16[1][0][0].set(6), std::out_of_range);
    EXPECT_THROW(d16[0][0][-1].set(0), std::out_of_range);

    EXPECT_EQ(callback_called, 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d2"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d2"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d2"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d3"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d4"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d5"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d6"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d6"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d6"_ss)).at(1), 2);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d6"_ss)).at(2), 3);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d6"_ss)).at(3), 4);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d6"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d7"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d8"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d9"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d9"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d9"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d10"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d10"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d10"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d11"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d11"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d11"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d12"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d12"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d12"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d13"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d13"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d13"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d15"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d15"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d15"_ss)).at(4), 5);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d16"_ss)).size(), 5u);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d16"_ss)).at(0), 1);
    EXPECT_EQ((*data_->value_store.getRecv(self_name, "d16"_ss)).at(4), 5);
}
// TEST_F(ValueTest, ArrayLike){
static_assert(
    std::is_same_v<traits::ElementTypeOf<std::vector<double>>, double>);
static_assert(
    std::is_same_v<traits::ElementTypeOf<std::array<double, 5>>, double>);
static_assert(std::is_same_v<traits::ElementTypeOf<double[5]>, double>);
static_assert(
    std::is_same_v<traits::ElementTypeOf<std::vector<std::vector<double>>>,
                   std::vector<double>>);
static_assert(
    std::is_same_v<traits::ElementTypeOf<std::array<std::vector<double>, 5>>,
                   std::vector<double>>);
static_assert(std::is_same_v<traits::ElementTypeOf<double[5][3]>, double[3]>);

static_assert(traits::IsArrayLike<std::vector<double>>::value);
static_assert(traits::IsNestedArrayLike<std::vector<double>>::value);
static_assert(traits::IsArrayLike<std::vector<int>>::value);
static_assert(traits::IsNestedArrayLike<std::vector<int>>::value);
static_assert(
    std::is_same_v<traits::ArrayLikeTrait<std::vector<double>>::ArrayLike,
                   std::nullptr_t>);
static_assert(
    std::is_same_v<traits::NestedArrayLikeTrait<std::vector<double>>::ArrayLike,
                   std::nullptr_t>);
static_assert(
    std::is_same_v<traits::ArrayLikeTrait<std::vector<int>>::ArrayLike,
                   std::nullptr_t>);
static_assert(
    std::is_same_v<traits::NestedArrayLikeTrait<std::vector<int>>::ArrayLike,
                   std::nullptr_t>);
static_assert(traits::IsArrayLike<std::array<double, 5>>::value);
static_assert(traits::IsNestedArrayLike<std::array<double, 5>>::value);
static_assert(traits::IsArrayLike<std::array<int, 5>>::value);
static_assert(traits::IsNestedArrayLike<std::array<int, 5>>::value);
static_assert(
    std::is_same_v<traits::ArrayLikeTrait<std::array<double, 5>>::ArrayLike,
                   std::nullptr_t>);
static_assert(std::is_same_v<
              traits::NestedArrayLikeTrait<std::array<double, 5>>::ArrayLike,
              std::nullptr_t>);
static_assert(
    std::is_same_v<traits::ArrayLikeTrait<std::array<int, 5>>::ArrayLike,
                   std::nullptr_t>);
static_assert(
    std::is_same_v<traits::NestedArrayLikeTrait<std::array<int, 5>>::ArrayLike,
                   std::nullptr_t>);
static_assert(traits::IsArrayLike<double[5]>::value);
static_assert(traits::IsNestedArrayLike<double[5]>::value);
static_assert(traits::IsArrayLike<int[5]>::value);
static_assert(traits::IsNestedArrayLike<int[5]>::value);
static_assert(traits::IsArrayLike<double (&)[5]>::value);
static_assert(traits::IsNestedArrayLike<double (&)[5]>::value);
static_assert(std::is_same_v<traits::ArrayLikeTrait<double[5]>::ArrayLike,
                             std::nullptr_t>);
static_assert(std::is_same_v<traits::NestedArrayLikeTrait<double[5]>::ArrayLike,
                             std::nullptr_t>);
static_assert(
    std::is_same_v<traits::ArrayLikeTrait<int[5]>::ArrayLike, std::nullptr_t>);
static_assert(std::is_same_v<traits::NestedArrayLikeTrait<int[5]>::ArrayLike,
                             std::nullptr_t>);
static_assert(std::is_same_v<traits::ArrayLikeTrait<double (&)[5]>::ArrayLike,
                             std::nullptr_t>);
static_assert(
    std::is_same_v<traits::NestedArrayLikeTrait<double (&)[5]>::ArrayLike,
                   std::nullptr_t>);

static_assert(!traits::IsArrayLike<std::vector<std::vector<double>>>::value);
static_assert(
    traits::IsNestedArrayLike<std::vector<std::vector<double>>>::value);
static_assert(
    !traits::IsArrayLike<std::array<std::array<double, 5>, 5>>::value);
static_assert(
    traits::IsNestedArrayLike<std::array<std::array<double, 5>, 5>>::value);
static_assert(!traits::IsArrayLike<double[5][5]>::value);
static_assert(traits::IsNestedArrayLike<double[5][5]>::value);

static_assert(traits::ArraySizeMatch<std::array<double, 5>, 5>::value);
static_assert(traits::NestedArraySizeMatch<std::array<double, 5>, 5>::value);
static_assert(traits::ArraySizeMatch<double[5], 5>::value);
static_assert(traits::NestedArraySizeMatch<double[5], 5>::value);
static_assert(!traits::ArraySizeMatch<std::array<double, 5>, 10>::value);
static_assert(!traits::NestedArraySizeMatch<std::array<double, 5>, 10>::value);
static_assert(!traits::ArraySizeMatch<double[5], 10>::value);
static_assert(!traits::NestedArraySizeMatch<double[5], 10>::value);
static_assert(std::is_same_v<traits::ArraySizeTrait<std::array<double, 5>,
                                                    5>::SizeMatchOrDynamic,
                             std::nullptr_t>);
static_assert(
    std::is_same_v<traits::ArraySizeTrait<double[5], 5>::SizeMatchOrDynamic,
                   std::nullptr_t>);
// vector -> always true (need runtime check)
static_assert(std::is_same_v<traits::ArraySizeTrait<std::vector<double>,
                                                    5>::SizeMatchOrDynamic,
                             std::nullptr_t>);
// }
static_assert(
    !traits::ArraySizeMatch<std::array<std::array<double, 5>, 5>, 25>::value);
static_assert(traits::NestedArraySizeMatch<std::array<std::array<double, 5>, 5>,
                                           25>::value);
static_assert(!traits::ArraySizeMatch<double[5][5], 25>::value);
static_assert(traits::NestedArraySizeMatch<double[5][5], 25>::value);
// static_assert(!traits::ArraySizeMatch<std::array<std::array<double, 5>, 5>,
// 5>::value);
static_assert(!traits::NestedArraySizeMatch<
              std::array<std::array<double, 5>, 5>, 5>::value);
static_assert(!traits::NestedArraySizeMatch<double[5][5], 5>::value);


TEST_F(ValueTest, valueGet) {
    data_->value_store.setRecv(
        "a"_ss, "b"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>({123})));
    EXPECT_EQ(value("a", "b").tryGet().value(), 123);
    EXPECT_EQ(value("a", "b").get(), 123);
    EXPECT_EQ(valueFixed<1>("a", "b").tryGet().value(), 123);
    EXPECT_EQ(valueFixed<1>("a", "b").get(), 123);
    EXPECT_EQ(value("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(value("a", "c").get(), 0);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(value(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->value_store.transferReq().count(self_name), 0u);
    value("a", "d").onChange(callback<Value>());
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}
TEST_F(ValueTest, valueGetVec) {
    data_->value_store.setRecv(
        "a"_ss, "b"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>({123})));
    EXPECT_EQ(value("a", "b").tryGetVec().value(), std::vector<double>{123});
    EXPECT_EQ(value("a", "b").getVec(), std::vector<double>{123});
    EXPECT_EQ(value("a", "c").tryGetVec(), std::nullopt);
    EXPECT_TRUE(value("a", "c").getVec().empty());
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(value(self_name, "b").tryGetVec(), std::nullopt);
    EXPECT_EQ(data_->value_store.transferReq().count(self_name), 0u);

    data_->value_store.setRecv("a"_ss, "b"_ss,
                               std::make_shared<std::vector<double>>(
                                   std::vector<double>({1, 2, 3, 4, 5})));
    EXPECT_EQ(value("a", "b").tryGetVec().value(),
              (std::vector<double>{1, 2, 3, 4, 5}));
    EXPECT_EQ(value("a", "b").getVec(), (std::vector<double>{1, 2, 3, 4, 5}));
    EXPECT_EQ(value("a", "b")[1].tryGet().value(), 2);
    EXPECT_EQ(value("a", "b")[1].get(), 2);
    EXPECT_EQ(value("a", "b")[5].tryGet(), std::nullopt);
    EXPECT_EQ(value("a", "b")[5].get(), 0);
}
