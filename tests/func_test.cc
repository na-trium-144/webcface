#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/func.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "webcface/message/message.h"

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

using namespace webcface;

static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(encoding::castToU8(std::string_view(str, len)));
}

class FuncTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
    FieldBase fieldBase(const SharedString &member,
                        std::string_view name) const {
        return FieldBase{member, SharedString(encoding::castToU8(name))};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{SharedString(encoding::castToU8(member)),
                         SharedString(encoding::castToU8(name))};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString(encoding::castToU8(name))};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, SharedString(encoding::castToU8(member)),
                     SharedString(encoding::castToU8(name))};
    }
    template <typename T1, typename T2>
    Func func(const T1 &member, const T2 &name) {
        return Func{field(member, name)};
    }
    template <typename T>
    AnonymousFunc afunc1(const T &func) {
        return AnonymousFunc{field(self_name, ""), func};
    }
    template <typename T>
    AnonymousFunc afunc2(const T &func) {
        return AnonymousFunc{func};
    }
};

TEST_F(FuncTest, valAdaptor) {
    EXPECT_FALSE(ValAdaptor(10).empty());
    EXPECT_EQ(static_cast<int>(ValAdaptor(10)), 10);
    EXPECT_EQ(static_cast<double>(ValAdaptor(10)), 10.0);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(0)), false);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(1)), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(2)), true);
    EXPECT_EQ(static_cast<std::string>(ValAdaptor(10)), "10");
    EXPECT_STREQ(static_cast<const char *>(ValAdaptor(10)), "10");
    EXPECT_EQ(static_cast<std::wstring>(ValAdaptor(10)), L"10");
    EXPECT_STREQ(static_cast<const wchar_t *>(ValAdaptor(10)), L"10");

    EXPECT_FALSE(ValAdaptor(1.5).empty());
    EXPECT_EQ(static_cast<int>(ValAdaptor(1.5)), 1);
    EXPECT_EQ(static_cast<double>(ValAdaptor(1.5)), 1.5);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(0.0)), false);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(1.0)), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(1.5)), true);
    EXPECT_EQ(static_cast<std::string>(ValAdaptor(1.5)), "1.500000");
    EXPECT_STREQ(static_cast<const char *>(ValAdaptor(1.5)), "1.500000");
    EXPECT_EQ(static_cast<std::wstring>(ValAdaptor(1.5)), L"1.500000");
    EXPECT_STREQ(static_cast<const wchar_t *>(ValAdaptor(1.5)), L"1.500000");

    EXPECT_FALSE(ValAdaptor(true).empty());
    EXPECT_EQ(static_cast<int>(ValAdaptor(true)), 1);
    EXPECT_EQ(static_cast<int>(ValAdaptor(false)), 0);
    EXPECT_EQ(static_cast<double>(ValAdaptor(true)), 1.0);
    EXPECT_EQ(static_cast<double>(ValAdaptor(false)), 0.0);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(true)), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor(false)), false);
    EXPECT_EQ(static_cast<std::string>(ValAdaptor(true)), "1");
    EXPECT_STREQ(static_cast<const char *>(ValAdaptor(true)), "1");
    EXPECT_EQ(static_cast<std::wstring>(ValAdaptor(true)), L"1");
    EXPECT_STREQ(static_cast<const wchar_t *>(ValAdaptor(true)), L"1");

    EXPECT_FALSE(ValAdaptor("1.5").empty());
    EXPECT_EQ(static_cast<int>(ValAdaptor("1.5")), 1);
    EXPECT_EQ(static_cast<double>(ValAdaptor("1.5")), 1.5);
    EXPECT_EQ(static_cast<bool>(ValAdaptor("")), false);
    EXPECT_EQ(static_cast<bool>(ValAdaptor("0")), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor("0.0")), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor("1.0")), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor("1.5")), true);
    EXPECT_EQ(static_cast<bool>(ValAdaptor("hoge")), true);
    EXPECT_EQ(static_cast<std::string>(ValAdaptor("1.5")), "1.5");
    EXPECT_STREQ(static_cast<const char *>(ValAdaptor("1.5")), "1.5");
    EXPECT_EQ(static_cast<std::wstring>(ValAdaptor("1.5")), L"1.5");
    EXPECT_STREQ(static_cast<const wchar_t *>(ValAdaptor("1.5")), L"1.5");

    EXPECT_TRUE(ValAdaptor().empty());
    EXPECT_TRUE(ValAdaptor("").empty());
    EXPECT_TRUE(ValAdaptor(L"").empty());
    EXPECT_TRUE(ValAdaptor(std::string("")).empty());
    EXPECT_TRUE(ValAdaptor(std::wstring(L"")).empty());

    EXPECT_TRUE(ValAdaptor(1) == ValAdaptor(1.0));
    EXPECT_TRUE(ValAdaptor(1) == 1);
    EXPECT_TRUE(1 == ValAdaptor(1));
    EXPECT_TRUE(ValAdaptor(1) == "1");
    EXPECT_TRUE(ValAdaptor(1) == L"1");
    EXPECT_TRUE("1" == ValAdaptor(1));
    EXPECT_TRUE(L"1" == ValAdaptor(1));
}
TEST_F(FuncTest, field) {
    EXPECT_EQ(func("a", "b").member().name(), "a");
    EXPECT_EQ(func("a", "b").name(), "b");

    EXPECT_THROW(Func().run(), std::runtime_error);
}
TEST_F(FuncTest, funcSet) {
    // 関数セットしreturnTypeとargsのチェック
    auto f = func(self_name, "a");
    f.set([]() {});
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"_ss))->return_type,
              ValType::none_);
    EXPECT_EQ(f.returnType(), ValType::none_);
    EXPECT_EQ(func(self_name, "a").returnType(), ValType::none_);
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"_ss))->args.size(), 0);
    EXPECT_EQ(f.args().size(), 0);
    EXPECT_EQ(func(self_name, "a").args().size(), 0);

    // 引数と戻り値をもつ関数
    f = func(self_name, "b");
    f.set([](int, double, bool, std::string) { return 0; });
    EXPECT_EQ(f.returnType(), ValType::int_);
    EXPECT_EQ(f.args().size(), 4);
    EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(2).type(), ValType::bool_);
    EXPECT_EQ(f.args(3).type(), ValType::string_);

    // 関数のパラメーター設定
    f.setArgs({Arg("0").init(1).min(0).max(2), Arg(L"1"),
               Arg("2").option({L"a", L"b", L"c"}),
               Arg("3").option({"a", "b", "c"})});
    EXPECT_EQ(f.args(0).name(), "0");
    EXPECT_EQ(f.args(0).nameW(), L"0");
    EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(f.args(0).init().value().as<double>(), 1);
    EXPECT_EQ(f.args(0).min().value(), 0);
    EXPECT_EQ(f.args(0).max().value(), 2);
    EXPECT_EQ(f.args(1).name(), "1");
    EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(1).init(), std::nullopt);
    EXPECT_EQ(f.args(1).min(), std::nullopt);
    EXPECT_EQ(f.args(1).max(), std::nullopt);
    ASSERT_EQ(f.args(2).option().size(), 3);
    EXPECT_EQ(f.args(2).option()[2], "c");
    EXPECT_EQ(f.args(2).option()[2], L"c");
    ASSERT_EQ(f.args(3).option().size(), 3);
    EXPECT_EQ(f.args(3).option()[2], "c");
    EXPECT_EQ(f.args(3).option()[2], L"c");

    // 未設定の関数呼び出しでエラー
    EXPECT_THROW(f.setArgs({}), std::invalid_argument);
    EXPECT_THROW(func(self_name, "c").setArgs({}), std::invalid_argument);
    EXPECT_THROW(func("a", "b").set([]() {}), std::invalid_argument);
}
TEST_F(FuncTest, funcRun) {
    // 引数と戻り値
    int called = 0;
    auto ret = func(self_name, "a")
                   .set([&](int a, double b, std::string c, bool d) {
                       EXPECT_EQ(a, 123);
                       EXPECT_EQ(b, 123.45);
                       EXPECT_EQ(c, "a");
                       EXPECT_TRUE(d);
                       ++called;
                       return 123.45;
                   })
                   .run(123, 123.45, "a", true);
    EXPECT_EQ(called, 1);
    EXPECT_EQ(static_cast<double>(ret), 123.45);
    called = 0;
    auto ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true);
    EXPECT_TRUE(ret_a.started.get());
    EXPECT_EQ(static_cast<double>(ret_a.result.get()), 123.45);
    EXPECT_EQ(ret_a.member().name(), self_name.decode());
    EXPECT_EQ(ret_a.name(), "a");
    EXPECT_EQ(called, 1);
    called = 0;

    // 引数の間違い
    EXPECT_THROW(func(self_name, "a").run(), std::invalid_argument);
    EXPECT_THROW(func(self_name, "a").runAsync().result.get(),
                 std::invalid_argument);
    EXPECT_TRUE(func(self_name, "a").runAsync().started.get());
    // 未設定関数の呼び出し
    EXPECT_THROW(func(self_name, "b").run(), FuncNotFound);
    EXPECT_THROW(func(self_name, "b").runAsync().result.get(), FuncNotFound);
    EXPECT_FALSE(func(self_name, "b").runAsync().started.get());
}
TEST_F(FuncTest, funcRunCond) {
    // setRunCondの動作確認
    int called = 0;
    auto add2 = [&](auto f, auto a) {
        std::cout << "add2 called" << std::endl;
        f(a);
        called |= 2;
        return ValAdaptor{};
    };
    // default
    data_->default_func_wrapper = add2;
    auto f = func(self_name, "a").set([&] {
        ++called;
        std::cout << "callback called" << std::endl;
    });
    std::cout << "run (cond=default add2) started" << std::endl;
    f.run();
    std::cout << "run (cond=default add2) finished" << std::endl;
    EXPECT_EQ(called, 3);

    // 各種wrapper
    called = 0;
    std::cout << "run (cond=None) started" << std::endl;
    f.setRunCondNone().run();
    std::cout << "run (cond=None) finished" << std::endl;
    EXPECT_EQ(called, 1);

    called = 0;
    std::cout << "run (cond=add2) started" << std::endl;
    f.setRunCond(add2).run();
    std::cout << "run (cond=add2) finished" << std::endl;
    EXPECT_EQ(called, 3);

    called = 0;
    static int counter_c = 0, counter_d = 0;
    struct A {
        A() { ++counter_c; }
        ~A() { ++counter_d; }
    };
    std::cout << "run (cond=ScopeGuard<A>) started" << std::endl;
    f.setRunCondScopeGuard<A>().run();
    std::cout << "run (cond=ScopeGuard<A>) finished" << std::endl;
    EXPECT_EQ(counter_c, 1);
    EXPECT_EQ(counter_d, 1);
    EXPECT_EQ(called, 1);

    called = 0;
    std::cout << "run (cond=OnSync) started" << std::endl;
    f.setRunCondOnSync().runAsync();
    wait();
    EXPECT_EQ(called, 0);
    auto fs = data_->func_sync_queue.pop();
    ASSERT_TRUE(fs.has_value());
    ASSERT_NE(*fs, nullptr);
    std::cout << "sync started" << std::endl;
    (*fs)->sync();
    std::cout << "sync finished" << std::endl;
    EXPECT_EQ(called, 1);
}
TEST_F(FuncTest, funcRunRemote) {
    func("a", "b").runAsync(1.23, true, "abc");
    bool call_msg_found = false;
    while (auto msg = data_->message_queue->pop()) {
        if (*msg == message::packSingle(FuncCall{
                        0,
                        0,
                        0,
                        "b"_ss,
                        {ValAdaptor(1.23), ValAdaptor(true),
                         ValAdaptor("abc")}}.toMessage())) {
            call_msg_found = true;
        }
    }
    EXPECT_TRUE(call_msg_found);
}

TEST_F(FuncTest, afuncSet1) {
    auto a = afunc1([](int) {});
    a.setArgs({Arg("a").init(0).min(-1).max(1)});
    auto f = func(self_name, "a");
    a.lockTo(f);
    EXPECT_EQ(f.args(0).name(), "a");
    EXPECT_EQ(static_cast<int>(*f.args(0).init()), 0);
    EXPECT_EQ(static_cast<int>(*f.args(0).min()), -1);
    EXPECT_EQ(static_cast<int>(*f.args(0).max()), 1);
}
TEST_F(FuncTest, afuncSet2) {
    auto a = afunc2([](int) {});
    // a.setArgs({Arg("a").init(0).min(-1).max(1)});
    auto f = func(self_name, "a");
    a.lockTo(f);
    // EXPECT_EQ(f.args(0).name(), "a");
    // EXPECT_EQ(static_cast<int>(*f.args(0).init()), 0);
    // EXPECT_EQ(static_cast<int>(*f.args(0).min()), -1);
    // EXPECT_EQ(static_cast<int>(*f.args(0).max()), 1);
}
