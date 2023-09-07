#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/func.h>
#include <stdexcept>

using namespace WebCFace;
class FuncTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<ClientData>(self_name);
    }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
    Func func(const std::string &member, const std::string &field) {
        return Func{Field{data_, member, field}};
    }
};

TEST_F(FuncTest, field) {
    EXPECT_EQ(func("a", "b").member().name(), "a");
    EXPECT_EQ(func("a", "b").name(), "b");
}
TEST_F(FuncTest, funcSet) {
    auto f = func(self_name, "a");
    f.set([]() {});
    EXPECT_EQ(data_->func_store.getRecv(self_name, "a")->return_type,
              ValType::none_);
    EXPECT_EQ(f.returnType(), ValType::none_);
    EXPECT_EQ(func(self_name, "a").returnType(), ValType::none_);
    EXPECT_EQ(data_->func_store.getRecv(self_name, "a")->args.size(), 0);
    EXPECT_EQ(f.args().size(), 0);
    EXPECT_EQ(func(self_name, "a").args().size(), 0);

    f = func(self_name, "b");
    f.set([](int, double, bool, std::string) { return 0; });
    EXPECT_EQ(f.returnType(), ValType::int_);
    EXPECT_EQ(f.args().size(), 4);
    EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(2).type(), ValType::bool_);
    EXPECT_EQ(f.args(3).type(), ValType::string_);

    f.setArgs({Arg("0").init(1).min(0).max(2), Arg("1"), Arg("2"),
               Arg("3").option({"a", "b", "c"})});
    EXPECT_EQ(f.args(0).name(), "0");
    EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(static_cast<double>(f.args(0).init().value()), 1);
    EXPECT_EQ(static_cast<double>(f.args(0).min().value()), 0);
    EXPECT_EQ(static_cast<double>(f.args(0).max().value()), 2);
    EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(1).init(), std::nullopt);
    EXPECT_EQ(f.args(1).min(), std::nullopt);
    EXPECT_EQ(f.args(1).max(), std::nullopt);
    EXPECT_EQ(f.args(3).option().size(), 3);

    EXPECT_THROW(f.setArgs({}), std::invalid_argument);
    EXPECT_THROW(func(self_name, "c").setArgs({}), std::invalid_argument);
    EXPECT_THROW(func("a", "b").set([]() {}), std::invalid_argument);
}
TEST_F(FuncTest, funcRun){
    int called = 0;
    auto ret = func(self_name, "a").set([&](int a, double b, std::string c, bool d){
        EXPECT_EQ(a, 123);
        EXPECT_EQ(b, 123.45);
        EXPECT_EQ(c, "a");
        EXPECT_TRUE(d);
        ++called;
        return 123.45;
    }).run(123, 123.45, "a", true);
    EXPECT_EQ(called, 1);
    EXPECT_EQ(static_cast<double>(ret), 123.45);
    called = 0;
    auto ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true);
    EXPECT_TRUE(ret_a.started.get());
    EXPECT_EQ(static_cast<double>(ret_a.result.get()), 123.45);
    EXPECT_EQ(ret_a.member().name(), self_name);
    EXPECT_EQ(ret_a.name(), "a");
    EXPECT_EQ(called, 1);
    called = 0;

    EXPECT_THROW(func(self_name, "a").run(), std::invalid_argument);
    EXPECT_THROW(func(self_name, "a").runAsync().result.get(), std::invalid_argument);
    EXPECT_TRUE(func(self_name, "a").runAsync().started.get());
    EXPECT_THROW(func(self_name, "b").run(), FuncNotFound);
    EXPECT_THROW(func(self_name, "b").runAsync().result.get(), FuncNotFound);
    EXPECT_FALSE(func(self_name, "b").runAsync().started.get());

    // todo: remote run
}
// todo: runCond
