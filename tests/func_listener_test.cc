#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/func.h>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace webcface;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif
static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class FuncListenerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
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
    Func func(const T1 &member, const T2 &name) {
        return Func{field(member, name)};
    }
    template <typename T1, typename T2>
    FuncListener funcListener(const T1 &member, const T2 &name) {
        return FuncListener{field(member, name)};
    }
};

TEST_F(FuncListenerTest, field) {
    EXPECT_EQ(funcListener("a", "b").member().name(), "a");
    EXPECT_EQ(funcListener("a", "b").name(), "b");
}
TEST_F(FuncListenerTest, listen) {
    auto f = func(self_name, "a");
    auto fl = funcListener(self_name, "a");
    fl.listen();
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"_ss))->return_type,
              ValType::none_);
    EXPECT_EQ(f.returnType(), ValType::none_);
    EXPECT_EQ(func(self_name, "a").returnType(), ValType::none_);
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"_ss))->args.size(), 0u);
    EXPECT_EQ(f.args().size(), 0u);
    EXPECT_EQ(func(self_name, "a").args().size(), 0u);

    // 引数と戻り値をもつ関数
    f = func(self_name, "b");
    fl = funcListener(self_name, "b");
    fl.listen(4, ValType::int_);
    EXPECT_EQ(f.returnType(), ValType::int_);
    EXPECT_EQ(f.args().size(), 4u);
    EXPECT_EQ(f.args(0).type(), ValType::none_);
    EXPECT_EQ(f.args(1).type(), ValType::none_);
    EXPECT_EQ(f.args(2).type(), ValType::none_);
    EXPECT_EQ(f.args(3).type(), ValType::none_);

    // 関数のパラメーター設定
    f = func(self_name, "c");
    fl = funcListener(self_name, "c");
    fl.setArgs({Arg("0").init(1).min(0).max(2), Arg("1"), Arg("2"),
                Arg("3").option({"a", "b", "c"})})
        .setReturnType(ValType::int_)
        .listen();
    EXPECT_EQ(f.args(0).name(), "0");
    // EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(static_cast<double>(f.args(0).init().value()), 1);
    EXPECT_EQ(static_cast<double>(f.args(0).min().value()), 0);
    EXPECT_EQ(static_cast<double>(f.args(0).max().value()), 2);
    // EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(1).init(), std::nullopt);
    EXPECT_EQ(f.args(1).min(), std::nullopt);
    EXPECT_EQ(f.args(1).max(), std::nullopt);
    EXPECT_EQ(f.args(3).option().size(), 3u);

    EXPECT_THROW(funcListener("a", "b").listen(), std::invalid_argument);
}
TEST_F(FuncListenerTest, funcRun) {
    // 引数と戻り値
    auto fl = funcListener(self_name, "a").listen(4);
    EXPECT_EQ(fl.fetchCall(), std::nullopt);

    auto ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true);
    EXPECT_TRUE(ret_a.reached());
    EXPECT_TRUE(ret_a.found());
    EXPECT_FALSE(ret_a.finished());
    wait();
    auto h = fl.fetchCall();
    ASSERT_NE(h, std::nullopt);
    EXPECT_EQ(static_cast<int>(h->args()[0]), 123);
    EXPECT_EQ(static_cast<double>(h->args()[1]), 123.45);
    EXPECT_EQ(static_cast<std::string>(h->args()[2]), "a");
    EXPECT_TRUE(static_cast<bool>(h->args()[3]));
    h->respond(123.45);
    EXPECT_TRUE(ret_a.finished());
    EXPECT_EQ(static_cast<double>(ret_a.response()), 123.45);

    ret_a = func(self_name, "a").runAsync(0, 0, 0, 0);
    wait();
    fl.fetchCall()->reject("aaa");
    EXPECT_TRUE(ret_a.finished());
    EXPECT_TRUE(ret_a.isError());
    EXPECT_EQ(ret_a.rejection(), "aaa");

    // 引数の間違い
    EXPECT_FALSE(func(self_name, "a").runAsync().rejection().empty());
    EXPECT_TRUE(func(self_name, "a").runAsync().found());
}
