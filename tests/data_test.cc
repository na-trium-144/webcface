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
    return SharedString::fromU8String(std::string_view(str, len));
}

class DataTest : public ::testing::Test {
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
    template <typename T1, typename T2>
    Text text(const T1 &member, const T2 &name) {
        return Text{field(member, name)};
    }
    Log log(const SharedString &member) { return Log{Field{data_, member}}; }
    Log log(std::string_view member) {
        return Log{Field{data_, SharedString::fromU8String(member)}};
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

    text("a", "b").onChange(callback<Text>());
    data_->text_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    log("a").onChange(callback<Log>());
    data_->log_append_event["a"_ss]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(DataTest, valueSet) {
    data_->value_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Value)>>(callback<Value>());
    value(self_name, "b").set(123);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "b"_ss)).at(0), 123);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(value("a", "b").set(123), std::invalid_argument);
}
TEST_F(DataTest, valueSetVec) {
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
    d6[3].set(4);
    d6[4].set(5);
    value(self_name, "d7").resize(5);
    EXPECT_EQ(callback_called, 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d"_ss)).at(4), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d2"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d2"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d2"_ss)).at(4), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d3"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d4"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d5"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(0), 1);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(1), 2);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(2), 3);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(3), 4);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d6"_ss)).at(4), 5);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d7"_ss)).size(), 5u);
    EXPECT_EQ((**data_->value_store.getRecv(self_name, "d8"_ss)).size(), 5u);
}
TEST_F(DataTest, textSet) {
    data_->text_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Text)>>(callback<Text>());
    text(self_name, "b").set("c");
    EXPECT_EQ(static_cast<std::string>(
                  **data_->text_store.getRecv(self_name, "b"_ss)),
              "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set("c"), std::invalid_argument);
}
TEST_F(DataTest, textSetW) {
    data_->text_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Text)>>(callback<Text>());
    text(self_name, "b").set(L"c");
    EXPECT_EQ(static_cast<std::string>(
                  **data_->text_store.getRecv(self_name, "b"_ss)),
              "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set(L"c"), std::invalid_argument);
}
TEST_F(DataTest, textSetV) {
    data_->text_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Text)>>(callback<Text>());
    text(self_name, "b").set(ValAdaptor(123));
    EXPECT_EQ(static_cast<std::string>(
                  **data_->text_store.getRecv(self_name, "b"_ss)),
              "123");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set(ValAdaptor(123)), std::invalid_argument);
}

TEST_F(DataTest, valueGet) {
    data_->value_store.setRecv(
        "a"_ss, "b"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>({123})));
    EXPECT_EQ(value("a", "b").tryGet().value(), 123);
    EXPECT_EQ(value("a", "b").get(), 123);
    EXPECT_EQ(value("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(value("a", "c").get(), 0);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(value(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->value_store.transferReq().count(self_name), 0u);
    value("a", "d").onChange(callback<Value>());
    EXPECT_EQ(data_->value_store.transferReq().at("a"_ss).at("d"_ss), 3u);
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
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(text(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->text_store.transferReq().count(self_name), 0u);
    text("a", "d").onChange(callback<Text>());
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}
TEST_F(DataTest, logGet) {
    using namespace std::chrono;
    auto logs =
        std::make_shared<std::vector<LogLineData>>(std::vector<LogLineData>{
            {1, system_clock::now(), "a"_ss},
            {2, system_clock::now(), "b"_ss},
            {3, system_clock::now(), "c"_ss},
        });
    data_->log_store.setRecv("a"_ss, logs);
    EXPECT_EQ(log("a").tryGet().value().size(), 3u);
    EXPECT_EQ(log("a").tryGetW().value().size(), 3u);
    ASSERT_EQ(log("a").get().size(), 3u);
    ASSERT_EQ(log("a").getW().size(), 3u);
    EXPECT_EQ(log("a").get()[2].level(), 3);
    EXPECT_EQ(log("a").getW()[2].level(), 3);
    EXPECT_EQ(log("a").get()[2].message(), "c");
    EXPECT_EQ(log("a").getW()[2].message(), L"c");
    EXPECT_EQ(log("b").tryGet(), std::nullopt);
    EXPECT_EQ(log("b").tryGetW(), std::nullopt);
    EXPECT_EQ(log("b").get().size(), 0u);
    EXPECT_EQ(log("b").getW().size(), 0u);
    EXPECT_EQ(data_->log_store.transferReq().at("a"_ss), true);
    EXPECT_EQ(data_->log_store.transferReq().at("b"_ss), true);
    ASSERT_TRUE(log(self_name).tryGet().has_value());
    ASSERT_TRUE(log(self_name).tryGetW().has_value());
    EXPECT_EQ(log(self_name).tryGet()->size(), 0u);
    EXPECT_EQ(log(self_name).tryGetW()->size(), 0u);
    EXPECT_EQ(data_->log_store.transferReq().count(self_name), 0u);
    log("d").onChange(callback<Log>());
    EXPECT_EQ(data_->log_store.transferReq().at("d"_ss), true);
}
TEST_F(DataTest, logClear) {
    using namespace std::chrono;
    auto logs =
        std::make_shared<std::vector<LogLineData>>(std::vector<LogLineData>{
            {1, system_clock::now(), "a"_ss},
            {2, system_clock::now(), "b"_ss},
            {3, system_clock::now(), "c"_ss},
        });
    data_->log_store.setRecv("a"_ss, logs);
    log("a").clear();
    EXPECT_EQ(log("a").tryGet().value().size(), 0u);
    EXPECT_EQ(log("a").tryGetW().value().size(), 0u);
}
// todo: hidden, free
