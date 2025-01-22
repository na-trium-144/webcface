#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/func.h"
#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/func.h>
#include <stdexcept>
#include <thread>

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
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

    EXPECT_THROW(Func().runAsync(), std::runtime_error);
}
TEST_F(FuncTest, funcSet) {
    // 関数セットしreturnTypeとargsのチェック
    auto f = func(self_name, "a");
    f.set([]() {});
    EXPECT_EQ(f.index(), 1);
    EXPECT_EQ(func(self_name, "a").index(), 1);
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"_ss))->return_type,
              ValType::none_);
    EXPECT_EQ(f.returnType(), ValType::none_);
    EXPECT_EQ(func(self_name, "a").returnType(), ValType::none_);
    EXPECT_EQ((*data_->func_store.getRecv(self_name, "a"_ss))->args->size(),
              0u);
    EXPECT_EQ(f.args().size(), 0u);
    EXPECT_EQ(func(self_name, "a").args().size(), 0u);

    // 引数と戻り値をもつ関数
    f = func(self_name, "b");
    f.set([](int, double, bool, const std::string &) { return 0; });
    EXPECT_EQ(f.index(), 2);
    EXPECT_EQ(func(self_name, "b").index(), 2);
    EXPECT_EQ(f.returnType(), ValType::int_);
    EXPECT_EQ(f.args().size(), 4u);
    EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(2).type(), ValType::bool_);
    EXPECT_EQ(f.args(3).type(), ValType::string_);

    // index
    f.setIndex(100);
    EXPECT_EQ(f.index(), 100);
    EXPECT_EQ(func(self_name, "b").index(), 100);

    // 関数のパラメーター設定
    f.setArgs({Arg("0").init(1).min(0).max(2), Arg(L"1"),
               Arg("2").option({L"a", L"b", L"c"}),
               Arg("3").option({"a", "b", "c"})});
    EXPECT_EQ(f.args(0).name(), "0");
    EXPECT_EQ(f.args(0).nameW(), L"0");
    EXPECT_EQ(f.args(0).type(), ValType::int_);
    EXPECT_EQ(f.args(0).init().value().asDouble(), 1);
    EXPECT_EQ(f.args(0).min().value(), 0);
    EXPECT_EQ(f.args(0).max().value(), 2);
    EXPECT_EQ(f.args(1).name(), "1");
    EXPECT_EQ(f.args(1).type(), ValType::double_);
    EXPECT_EQ(f.args(1).init(), std::nullopt);
    EXPECT_EQ(f.args(1).min(), std::nullopt);
    EXPECT_EQ(f.args(1).max(), std::nullopt);
    ASSERT_EQ(f.args(2).option().size(), 3u);
    EXPECT_EQ(f.args(2).option()[2], "c");
    EXPECT_EQ(f.args(2).option()[2], L"c");
    ASSERT_EQ(f.args(3).option().size(), 3u);
    EXPECT_EQ(f.args(3).option()[2], "c");
    EXPECT_EQ(f.args(3).option()[2], L"c");
    
    EXPECT_THROW(f.setArgs({}), std::invalid_argument);

    // 未設定の関数呼び出しでエラー
    EXPECT_THROW(func(self_name, "c").setIndex(1), std::invalid_argument);
    EXPECT_THROW(func(self_name, "c").setArgs({}), std::invalid_argument);
    EXPECT_THROW(func("a", "b").set([]() {}), std::invalid_argument);
}
struct CopyCounter {
    mutable int c = 0;
    CopyCounter() = default;
    CopyCounter(const CopyCounter &other) : c(other.c + 1) {}
    CopyCounter &operator=(const CopyCounter &other) {
        this->c = other.c + 1;
        return *this;
    }
    CopyCounter(CopyCounter &&) = default;
    CopyCounter &operator=(CopyCounter &&) = default;
    int operator()() const { return c; }
};
void hoge() {}
TEST_F(FuncTest, funcSetCopy) {
    EXPECT_EQ(func(self_name, "a")
                  .set(CopyCounter())
                  .runAsync()
                  .waitFinish()
                  .response()
                  .asInt(),
              0);
    EXPECT_EQ(func(self_name, "a")
                  .setAsync(CopyCounter())
                  .runAsync()
                  .waitFinish()
                  .response()
                  .asInt(),
              0);
    std::function<int()> copy_counter = CopyCounter();
    EXPECT_EQ(func(self_name, "a")
                  .set(std::move(copy_counter))
                  .runAsync()
                  .waitFinish()
                  .response()
                  .asInt(),
              0);
    copy_counter = CopyCounter();
    EXPECT_EQ(func(self_name, "a")
                  .setAsync(std::move(copy_counter))
                  .runAsync()
                  .waitFinish()
                  .response()
                  .asInt(),
              0);
    // ついでに関数ポインタでもビルドできることを確認
    func(self_name, "a").set(hoge);
    func(self_name, "a").setAsync(hoge);
}
TEST_F(FuncTest, funcRun) {
    // 引数と戻り値
    int called = 0;
    auto main_id = std::this_thread::get_id();
    func(self_name, "a")
        .set([&](int a, double b, const std::string &c, bool d) {
            if (a == 0) {
                throw std::invalid_argument("a == 0");
            }
            EXPECT_EQ(a, 123);
            EXPECT_EQ(b, 123.45);
            EXPECT_EQ(c, "a");
            EXPECT_TRUE(d);
            ++called;
            // setしてるのでrunAsyncしてもmainスレッドで実行される
            EXPECT_EQ(std::this_thread::get_id(), main_id);
            return 123.45;
        });
    auto ret_a = func(self_name, "a").runAsync(0, 123.45, "a", true);
    EXPECT_TRUE(ret_a.isError());
    EXPECT_TRUE(ret_a.response().empty());
    EXPECT_EQ(ret_a.rejection(), "a == 0");
    EXPECT_EQ(ret_a.rejectionW(), L"a == 0");
    ret_a.onReach([&](const Promise &p) {
        called++;
        EXPECT_TRUE(p.found());
    });
    ret_a.onFinish([&](const Promise &p) {
        called++;
        EXPECT_TRUE(p.isError());
        EXPECT_TRUE(p.response().empty());
        EXPECT_EQ(p.rejection(), "a == 0");
        EXPECT_EQ(p.rejectionW(), L"a == 0");
    });
    EXPECT_EQ(called, 2);
    called = 0;
    ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true);
    EXPECT_TRUE(ret_a.found());
    EXPECT_FALSE(ret_a.isError());
    EXPECT_EQ(static_cast<double>(ret_a.response()), 123.45);
    EXPECT_EQ(ret_a.member().name(), self_name.decode());
    EXPECT_EQ(ret_a.name(), "a");
    EXPECT_EQ(called, 1);
    called = 0;
    ret_a.onReach([&](const Promise &p) {
        called++;
        EXPECT_TRUE(p.found());
    });
    ret_a.onFinish([&](const Promise &p) {
        called++;
        EXPECT_FALSE(ret_a.isError());
        EXPECT_EQ(static_cast<double>(p.response()), 123.45);
    });
    EXPECT_EQ(called, 2);
    called = 0;

    // 引数の間違い
    EXPECT_FALSE(func(self_name, "a").runAsync().rejection().empty());
    EXPECT_TRUE(func(self_name, "a").runAsync().found());
    // 未設定関数の呼び出し
    EXPECT_FALSE(func(self_name, "b").runAsync().found());
    EXPECT_FALSE(func(self_name, "b").runAsync().rejection().empty());
}
TEST_F(FuncTest, funcAsyncRun) {
    // 引数と戻り値
    int called = 0;
    auto main_id = std::this_thread::get_id();

    func(self_name, "a")
        .setAsync([&](int a, double b, const std::string &c, bool d) {
            if (a == 0) {
                throw std::invalid_argument("a == 0");
            }
            EXPECT_EQ(a, 123);
            EXPECT_EQ(b, 123.45);
            EXPECT_EQ(c, "a");
            EXPECT_TRUE(d);
            ++called;
            // setAsyncなので別スレッド
            EXPECT_NE(std::this_thread::get_id(), main_id);
            return 123.45;
        });
    auto ret_a = func(self_name, "a").runAsync(0, 123.45, "a", true);
    ret_a.waitFinish();
    EXPECT_TRUE(ret_a.isError());
    EXPECT_TRUE(ret_a.response().empty());
    EXPECT_EQ(ret_a.rejection(), "a == 0");
    EXPECT_EQ(ret_a.rejectionW(), L"a == 0");
    ret_a.onReach([&](const Promise &p) {
        called++;
        EXPECT_TRUE(p.found());
    });
    ret_a.onFinish([&](const Promise &p) {
        called++;
        EXPECT_TRUE(p.isError());
        EXPECT_TRUE(p.response().empty());
        EXPECT_EQ(p.rejection(), "a == 0");
        EXPECT_EQ(p.rejectionW(), L"a == 0");
    });
    EXPECT_EQ(called, 2);
    called = 0;
    ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true).waitFinish();
    EXPECT_EQ(called, 1);
    EXPECT_TRUE(ret_a.found());
    EXPECT_FALSE(ret_a.isError());
    EXPECT_EQ(static_cast<double>(ret_a.response()), 123.45);
    EXPECT_EQ(ret_a.member().name(), self_name.decode());
    EXPECT_EQ(ret_a.name(), "a");
    EXPECT_EQ(called, 1);
    called = 0;
    ret_a.onReach([&](const Promise &p) {
        called++;
        EXPECT_TRUE(p.found());
    });
    ret_a.onFinish([&](const Promise &p) {
        called++;
        EXPECT_FALSE(ret_a.isError());
        EXPECT_EQ(static_cast<double>(p.response()), 123.45);
    });
    EXPECT_EQ(called, 2);
    called = 0;

    // 引数の間違い
    EXPECT_FALSE(func(self_name, "a").runAsync().rejection().empty());
    EXPECT_TRUE(func(self_name, "a").runAsync().found());
}
TEST_F(FuncTest, funcHandleRun) {
    // 引数と戻り値
    auto main_id = std::this_thread::get_id();
    webcface::CallHandle handle_copy;
    func(self_name, "a")
        .set([&](const webcface::CallHandle &h) {
            if (h.args().at(0) == 0) {
                throw std::invalid_argument("a == 0");
            }
            if (h.args().at(0) == 1) {
                h.reject("a == 1");
                return;
            }
            EXPECT_EQ(h.args().at(0).asInt(), 123);
            EXPECT_EQ(h.args().at(1).asDouble(), 123.45);
            EXPECT_EQ(h.args().at(2).asString(), "a");
            EXPECT_TRUE(h.args().at(3).asBool());
            // setしてるのでrunAsyncしてもmainスレッドで実行される
            EXPECT_EQ(std::this_thread::get_id(), main_id);
            handle_copy = h;
            return;
        })
        .setArgs({Arg(), Arg(), Arg(), Arg()});
    auto ret_a = func(self_name, "a").runAsync(0, 123.45, "a", true);
    EXPECT_TRUE(ret_a.isError());
    EXPECT_EQ(ret_a.rejection(), "a == 0");
    ret_a = func(self_name, "a").runAsync(1, 123.45, "a", true);
    EXPECT_TRUE(ret_a.isError());
    EXPECT_EQ(ret_a.rejection(), "a == 1");
    ret_a = func(self_name, "a").runAsync(2);
    EXPECT_TRUE(ret_a.isError());
    EXPECT_FALSE(ret_a.rejection().empty());

    ret_a = func(self_name, "a").runAsync(123, 123.45, "a", true);
    EXPECT_TRUE(ret_a.found());
    EXPECT_FALSE(ret_a.finished());
    handle_copy.respond(123.45);
    EXPECT_TRUE(ret_a.finished());
    EXPECT_FALSE(ret_a.isError());
    EXPECT_EQ(static_cast<double>(ret_a.response()), 123.45);
}
TEST_F(FuncTest, funcRunRemote) {
    // 未接続
    auto res = func("a", "b").runAsync(1.23, true, "abc");
    EXPECT_TRUE(res.reached());
    EXPECT_TRUE(res.finished());
    EXPECT_FALSE(res.found());
    EXPECT_TRUE(res.isError());
    {
        internal::ClientData::ScopedWsLock lock_ws(data_);
        EXPECT_TRUE(lock_ws.getData().sync_queue.empty());
        lock_ws.getData().connected = true;
    }
    res = func("a", "b").runAsync(1.23, true, "abc");
    EXPECT_FALSE(res.reached());
    bool call_msg_found = false;
    {
        internal::ClientData::ScopedWsLock lock_ws(data_);
        while (!lock_ws.getData().sync_queue.empty()) {
            auto msg = lock_ws.getData().sync_queue.front();
            lock_ws.getData().sync_queue.pop();
            if (std::get<0>(msg) ==
                message::packSingle(message::Call{
                    1,
                    0,
                    0,
                    "b"_ss,
                    {ValAdaptor(1.23), ValAdaptor(true), ValAdaptor("abc")}})) {
                call_msg_found = true;
            }
        }
    }
    EXPECT_TRUE(call_msg_found);
}
