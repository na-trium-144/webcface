#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include "webcface/field.h"
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(Encoding::castToU8(std::string_view(str, len)));
}

class DataTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<Internal::ClientData> data_;
    FieldBase fieldBase(const SharedString &member,
                        std::string_view name) const {
        return FieldBase{member, SharedString(Encoding::castToU8(name))};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{SharedString(Encoding::castToU8(member)),
                         SharedString(Encoding::castToU8(name))};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString(Encoding::castToU8(name))};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, SharedString(Encoding::castToU8(member)),
                     SharedString(Encoding::castToU8(name))};
    }
    template <typename T1, typename T2>
    Value value(const T1 &member, const T2 &name) {
        return Value{field(member, name)};
    }
    template <typename T1, typename T2>
    Text text(const T1 &member, const T2 &name) {
        return Text{field(member, name)};
    }
    Log log(const SharedString &member) { return Log{Field{data_, member}}; }
    Log log(std::string_view member) {
        return Log{Field{data_, SharedString(Encoding::castToU8(member))}};
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

TEST_F(DataTest, field) {
    EXPECT_EQ(value("a", "b").member().name(), "a");
    EXPECT_EQ(value("a", "b").member().nameW(), L"a");
    EXPECT_EQ(value("a", "b").name(), "b");
    EXPECT_EQ(value("a", "b").nameW(), L"b");
    EXPECT_EQ(value("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(value("a", "b").child(L"c").name(), "b.c");
    EXPECT_EQ(value("a", "b.c").parent().name(), "b");
    EXPECT_EQ(text("a", "b").member().name(), "a");
    EXPECT_EQ(text("a", "b").name(), "b");
    EXPECT_EQ(text("a", "b").nameW(), L"b");
    EXPECT_EQ(text("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(text("a", "b").child(L"c").name(), "b.c");
    EXPECT_EQ(log("a").member().name(), "a");

    EXPECT_THROW(Value().tryGet(), std::runtime_error);
    EXPECT_THROW(Text().tryGet(), std::runtime_error);
    EXPECT_THROW(Log().tryGet(), std::runtime_error);
}
TEST_F(DataTest, eventTarget) {
    auto handle = value("a", "b").appendListener(callback<Value>());
    data_->value_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    value("a", "b").removeListener(handle);
    data_->value_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 0);
    value("a", "b").appendListener(callbackVoid());
    data_->value_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;

    text("a", "b").appendListener(callback<Text>());
    data_->text_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    log("a").appendListener(callback<Log>());
    data_->log_append_event["a"_ss]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(DataTest, valueSet) {
    (data_->value_change_event[self_name]["b"_ss] =
         std::make_shared<eventpp::CallbackList<void(Value)>>())
        ->append(callback<Value>());
    value(self_name, "b").set(123);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "b"_ss), 123);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(value("a", "b").set(123), std::invalid_argument);
}
TEST_F(DataTest, valueSetVec) {
    (data_->value_change_event[self_name]["d"_ss] =
         std::make_shared<eventpp::CallbackList<void(Value)>>())
        ->append(callback<Value>());
    value(self_name, "d").set({1, 2, 3, 4, 5});
    value(self_name, "d2").set(std::vector<double>{1, 2, 3, 4, 5});
    value(self_name, "d3").set(std::vector<int>{1, 2, 3, 4, 5});
    value(self_name, "d4").set(std::array<int, 5>{1, 2, 3, 4, 5});
    int a[5] = {1, 2, 3, 4, 5};
    value(self_name, "d5").set(a);
    auto d6 = value(self_name, "d6");
    d6.set(1);
    d6.push_back(2);
    d6.push_back(3);
    d6[3].set(4);
    d6[4].set(5);
    value(self_name, "d7").resize(5);
    EXPECT_EQ(callback_called, 1);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d"_ss), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).size(), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).at(4), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d2"_ss)).size(), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d2"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d2"_ss)).at(4), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d3"_ss)).size(), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d4"_ss)).size(), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d5"_ss)).size(), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).size(), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(1), 2);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(2), 3);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(3), 4);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(4), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d7"_ss)).size(), 5);
}
TEST_F(DataTest, textSet) {
    (data_->text_change_event[self_name]["b"_ss] =
         std::make_shared<eventpp::CallbackList<void(Text)>>())
        ->append(callback<Text>());
    text(self_name, "b").set("c");
    EXPECT_EQ(static_cast<std::string>(
                  **data_->text_store.getRecv(self_name, "b"_ss)),
              "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set("c"), std::invalid_argument);
}
TEST_F(DataTest, textSetW) {
    (data_->text_change_event[self_name]["b"_ss] =
         std::make_shared<eventpp::CallbackList<void(Text)>>())
        ->append(callback<Text>());
    text(self_name, "b").set(L"c");
    EXPECT_EQ(static_cast<std::string>(
                  **data_->text_store.getRecv(self_name, "b"_ss)),
              "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set(L"c"), std::invalid_argument);
}
TEST_F(DataTest, textSetV) {
    (data_->text_change_event[self_name]["b"_ss] =
         std::make_shared<eventpp::CallbackList<void(Text)>>())
        ->append(callback<Text>());
    text(self_name, "b").set(ValAdaptor(123));
    EXPECT_EQ(static_cast<std::string>(
                  **data_->text_store.getRecv(self_name, "b"_ss)),
              "123");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set(ValAdaptor(123)), std::invalid_argument);
}

TEST_F(DataTest, dict) {
    // DictElement<std::shared_ptr<VectorOpt<double>>> e{"a", {1, 2}};
    // EXPECT_EQ(e.value, std::nullopt);
    // EXPECT_EQ(e.children.size(), 2);
    // EXPECT_EQ(*e.children[0].value.value(), 1);
    Value::Dict a{{"a", 1},
                  {"b", 2},
                  {"c", {{"a", 1}, {"b", 2}}},
                  {"v", {1, 2, 3, 4, 5}}};
    EXPECT_FALSE(a.hasValue());
    EXPECT_TRUE(a["a"].hasValue());
    EXPECT_EQ(a["a"].get(), 1);
    EXPECT_EQ(a["c"]["a"].get(), 1);
    EXPECT_EQ(a["v"].getVec().size(), 5);
    EXPECT_EQ(a["v"].getVec().at(0), 1);
}
TEST_F(DataTest, valueSetDict) {
    (data_->value_change_event[self_name]["d"_ss] =
         std::make_shared<eventpp::CallbackList<void(Value)>>())
        ->append(callback<Value>());
    value(self_name, "d").set({{"a", 100}});
    // dにはセットしてないのでeventは発動しない
    EXPECT_EQ(callback_called, 0);

    (data_->value_change_event[self_name]["d.a"_ss] =
         std::make_shared<eventpp::CallbackList<void(Value)>>())
        ->append(callback<Value>());
    value(self_name, "d")
        .set({{"a", 1},
              {"b", 2},
              {"c", {{"a", 1}, {"b", 2}}},
              {"v", {1, 2, 3, 4, 5}}});
    // d.aではevent発動する
    EXPECT_EQ(callback_called, 1);
    // 値がセットされている
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.a"_ss), 1);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.b"_ss), 2);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.c.a"_ss), 1);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.c.b"_ss), 2);
    // 1つの値として取得した場合1つ目の要素
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.v"_ss), 1);
    EXPECT_EQ(static_cast<std::vector<double>>(
                  **data_->value_store.getRecv(self_name, "d.v"_ss))
                  .size(),
              5);
}
/*TEST_F(DataTest, textSetDict) {
    data_->text_change_event.appendListener(FieldBase{self_name, "d"},
                                            callback());
    text(self_name, "d").set({{"a", ""}});
    // dにはセットしてないのでeventは発動しない
    EXPECT_EQ(callback_called, 0);

    data_->text_change_event.appendListener(FieldBase{self_name, "d.a"},
                                            callback());
    text(self_name, "d")
        .set({{"a", "1"}, {"b", "2"}, {"c", {{"a", "1"}, {"b", "2"}}}});
    // d.aではevent発動する
    EXPECT_EQ(callback_called, 1);
    // 値がセットされている
    EXPECT_EQ(**data_->text_store.getRecv(self_name, "d.a"), "1");
    EXPECT_EQ(**data_->text_store.getRecv(self_name, "d.b"), "2");
    EXPECT_EQ(**data_->text_store.getRecv(self_name, "d.c.a"), "1");
    EXPECT_EQ(**data_->text_store.getRecv(self_name, "d.c.b"), "2");
}
*/
TEST_F(DataTest, valueGet) {
    data_->value_store.setRecv("a"_ss, "b"_ss,
                               std::make_shared<VectorOpt<double>>(123));
    EXPECT_EQ(value("a", "b").tryGet().value(), 123);
    EXPECT_EQ(value("a", "b").get(), 123);
    EXPECT_EQ(value("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(value("a", "c").get(), 0);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("b"_ss), 1);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("c"_ss), 2);
    EXPECT_EQ(value(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->value_store.transferReq().count(self_name), 0);
    value("a", "d").appendListener(callback<Value>());
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("d"_ss), 3);
}
TEST_F(DataTest, valueGetDict) {
    data_->value_store.setRecv("a"_ss, "d.a"_ss,
                               std::make_shared<VectorOpt<double>>(1));
    data_->value_store.setRecv("a"_ss, "d.b"_ss,
                               std::make_shared<VectorOpt<double>>(2));
    data_->value_store.setRecv("a"_ss, "d.c.a"_ss,
                               std::make_shared<VectorOpt<double>>(1));
    data_->value_store.setRecv("a"_ss, "d.c.b"_ss,
                               std::make_shared<VectorOpt<double>>(2));
    data_->value_store.setRecv("a"_ss, "d.v"_ss,
                               std::make_shared<VectorOpt<double>>(
                                   std::vector<double>{1, 2, 3, 4, 5}));
    EXPECT_NE(value("a", "d").tryGetRecurse(), std::nullopt);
    EXPECT_EQ(value("a", "d").tryGet(), std::nullopt);
    EXPECT_EQ(value("a", "d.a").tryGetRecurse(), std::nullopt);
    EXPECT_EQ(value("a", "a").tryGetRecurse(), std::nullopt);
    auto d = value("a", "d").getRecurse();
    EXPECT_EQ(d["a"].get(), 1);
    EXPECT_EQ(d["b"].get(), 2);
    EXPECT_EQ(d["c"]["a"].get(), 1);
    EXPECT_EQ(d["c"]["b"].get(), 2);
    EXPECT_EQ(d["c.a"].get(), 1);
    EXPECT_EQ(d["v"].get(), 1);
    EXPECT_EQ(d["v"].getVec().size(), 5);
    EXPECT_EQ(d["v"].getVec().at(0), 1);
    EXPECT_EQ(d["v"].getVec().at(4), 5);
}
TEST_F(DataTest, textGet) {
    data_->text_store.setRecv("a"_ss, "b"_ss,
                              std::make_shared<ValAdaptor>("hoge"));
    ASSERT_NE(text("a", "b").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "b").tryGet().value(), "hoge");
    EXPECT_EQ(text("a", "b").tryGetW().value(), L"hoge");
    EXPECT_EQ(text("a", "b").tryGetV().value(), ValAdaptor("hoge"));
    EXPECT_EQ(text("a", "b").get(), "hoge");
    EXPECT_EQ(text("a", "b").getW(), L"hoge");
    EXPECT_EQ(text("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "c").tryGetW(), std::nullopt);
    EXPECT_EQ(text("a", "c").tryGetV(), std::nullopt);
    EXPECT_EQ(text("a", "c").get(), "");
    EXPECT_EQ(text("a", "c").getW(), L"");
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("b"_ss), 1);
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("c"_ss), 2);
    EXPECT_EQ(text(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->text_store.transferReq().count(self_name), 0);
    text("a", "d").appendListener(callback<Text>());
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("d"_ss), 3);
}
/*TEST_F(DataTest, textGetDict) {
    data_->text_store.setRecv("a", "d.a", std::make_shared<std::string>("1"));
    data_->text_store.setRecv("a", "d.b", std::make_shared<std::string>("2"));
    data_->text_store.setRecv("a", "d.c.a", std::make_shared<std::string>("1"));
    data_->text_store.setRecv("a", "d.c.b", std::make_shared<std::string>("2"));
    EXPECT_NE(text("a", "d").tryGetRecurse(), std::nullopt);
    EXPECT_EQ(text("a", "d").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "d.a").tryGetRecurse(), std::nullopt);
    EXPECT_EQ(text("a", "a").tryGetRecurse(), std::nullopt);
    auto d = text("a", "d").getRecurse();
    EXPECT_EQ(d["a"].get(), "1");
    EXPECT_EQ(d["b"].get(), "2");
    EXPECT_EQ(d["c"]["a"].get(), "1");
    EXPECT_EQ(d["c"]["b"].get(), "2");
    EXPECT_EQ(d["c.a"].get(), "1");
}
*/
TEST_F(DataTest, logGet) {
    using namespace std::chrono;
    auto logs =
        std::make_shared<std::vector<LogLineData<>>>(std::vector<LogLineData<>>{
            {1, system_clock::now(), "a"_ss},
            {2, system_clock::now(), "b"_ss},
            {3, system_clock::now(), "c"_ss},
        });
    data_->log_store->setRecv("a"_ss, logs);
    EXPECT_EQ(log("a").tryGet().value().size(), 3);
    EXPECT_EQ(log("a").tryGetW().value().size(), 3);
    ASSERT_EQ(log("a").get().size(), 3);
    ASSERT_EQ(log("a").getW().size(), 3);
    EXPECT_EQ(log("a").get()[2].level(), 3);
    EXPECT_EQ(log("a").getW()[2].level(), 3);
    EXPECT_EQ(log("a").get()[2].message(), "c");
    EXPECT_EQ(log("a").getW()[2].message(), L"c");
    EXPECT_EQ(log("b").tryGet(), std::nullopt);
    EXPECT_EQ(log("b").tryGetW(), std::nullopt);
    EXPECT_EQ(log("b").get().size(), 0);
    EXPECT_EQ(log("b").getW().size(), 0);
    EXPECT_EQ(data_->log_store->transferReq().at("a"_ss), true);
    EXPECT_EQ(data_->log_store->transferReq().at("b"_ss), true);
    ASSERT_TRUE(log(self_name).tryGet().has_value());
    ASSERT_TRUE(log(self_name).tryGetW().has_value());
    EXPECT_EQ(log(self_name).tryGet()->size(), 0);
    EXPECT_EQ(log(self_name).tryGetW()->size(), 0);
    EXPECT_EQ(data_->log_store->transferReq().count(self_name), 0);
    log("d").appendListener(callback<Log>());
    EXPECT_EQ(data_->log_store->transferReq().at("d"_ss), true);
}
TEST_F(DataTest, logClear) {
    using namespace std::chrono;
    auto logs =
        std::make_shared<std::vector<LogLineData<>>>(std::vector<LogLineData<>>{
            {1, system_clock::now(), "a"_ss},
            {2, system_clock::now(), "b"_ss},
            {3, system_clock::now(), "c"_ss},
        });
    data_->log_store->setRecv("a"_ss, logs);
    log("a").clear();
    EXPECT_EQ(log("a").tryGet().value().size(), 0);
    EXPECT_EQ(log("a").tryGetW().value().size(), 0);
}
// todo: hidden, free
