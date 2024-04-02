#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/func.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "../message/message.h"

using namespace webcface;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif
static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}

class FuncListenerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    Func func(const std::string &member, const std::string &field) {
        return Func{Field{data_, member, field}};
    }
    FuncListener funcListener(const std::string &member,
                              const std::string &field) {
        return FuncListener{Field{data_, member, field}};
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
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"))->return_type,
              ValType::none_);
    EXPECT_EQ(f.returnType(), ValType::none_);
    EXPECT_EQ(func(self_name, "a").returnType(), ValType::none_);
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"))->args.size(), 0);
    EXPECT_EQ(f.args().size(), 0);
    EXPECT_EQ(func(self_name, "a").args().size(), 0);

    // 引数と戻り値をもつ関数
    f = func(self_name, "b");
    fl = funcListener(self_name, "b");
    fl.listen(4, ValType::int_);
    EXPECT_EQ(f.returnType(), ValType::int_);
    EXPECT_EQ(f.args().size(), 4);
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
    EXPECT_EQ(f.args(3).option().size(), 3);

    EXPECT_THROW(funcListener("a", "b").listen(), std::invalid_argument);
}
TEST_F(FuncListenerTest, funcRun) {
    // 引数と戻り値
    auto fl = funcListener(self_name, "a").listen(4);
    EXPECT_EQ(fl.fetchCall(), std::nullopt);

    auto ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true);
    EXPECT_TRUE(ret_a.started.get());
    wait();
    auto h = fl.fetchCall();
    ASSERT_NE(h, std::nullopt);
    EXPECT_EQ(static_cast<int>(h->args()[0]), 123);
    EXPECT_EQ(static_cast<double>(h->args()[1]), 123.45);
    EXPECT_EQ(static_cast<std::string>(h->args()[2]), "a");
    EXPECT_TRUE(static_cast<bool>(h->args()[3]));
    h->respond(123.45);
    EXPECT_EQ(static_cast<double>(ret_a.result.get()), 123.45);

    ret_a = func(self_name, "a").runAsync(0, 0, 0, 0);
    wait();
    fl.fetchCall()->reject("aaa");
    EXPECT_THROW(ret_a.result.get(), std::runtime_error);

    // 引数の間違い
    EXPECT_THROW(func(self_name, "a").run(), std::invalid_argument);
    EXPECT_THROW(func(self_name, "a").runAsync().result.get(),
                 std::invalid_argument);
    EXPECT_TRUE(func(self_name, "a").runAsync().started.get());
}
