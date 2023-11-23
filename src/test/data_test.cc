#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
class DataTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    Value value(const std::string &member, const std::string &field) {
        return Value{Field{data_, member, field}};
    }
    Text text(const std::string &member, const std::string &field) {
        return Text{Field{data_, member, field}};
    }
    Log log(const std::string &member) { return Log{Field{data_, member}}; }
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(DataTest, field) {
    EXPECT_EQ(value("a", "b").member().name(), "a");
    EXPECT_EQ(value("a", "b").name(), "b");
    EXPECT_EQ(value("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(text("a", "b").member().name(), "a");
    EXPECT_EQ(text("a", "b").name(), "b");
    EXPECT_EQ(text("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(log("a").member().name(), "a");

    EXPECT_THROW(Value().tryGet(), std::runtime_error);
    EXPECT_THROW(Text().tryGet(), std::runtime_error);
    EXPECT_THROW(Log().tryGet(), std::runtime_error);
}
TEST_F(DataTest, eventTarget) {
    value("a", "b").appendListener(callback<Value>());
    data_->value_change_event.dispatch(FieldBase{"a", "b"},
                                       Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    text("a", "b").appendListener(callback<Text>());
    data_->text_change_event.dispatch(FieldBase{"a", "b"},
                                      Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    log("a").appendListener(callback<Log>());
    data_->log_append_event.dispatch("a", Field{data_, "a"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(DataTest, valueSet) {
    data_->value_change_event.appendListener(FieldBase{self_name, "b"},
                                             callback());
    value(self_name, "b").set(123);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "b"), 123);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(value("a", "b").set(123), std::invalid_argument);
}
TEST_F(DataTest, valueSetVec) {
    data_->value_change_event.appendListener(FieldBase{self_name, "d"},
                                             callback());
    value(self_name, "d").set({1, 2, 3, 4, 5});
    EXPECT_EQ(callback_called, 1);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d"), 1);
    EXPECT_EQ(static_cast<std::vector<double>>(
                  **data_->value_store.getRecv(self_name, "d"))
                  .size(),
              5);
    EXPECT_EQ(static_cast<std::vector<double>>(
                  **data_->value_store.getRecv(self_name, "d"))
                  .at(0),
              1);
    EXPECT_EQ(static_cast<std::vector<double>>(
                  **data_->value_store.getRecv(self_name, "d"))
                  .at(4),
              5);
}
TEST_F(DataTest, textSet) {
    data_->text_change_event.appendListener(FieldBase{self_name, "b"},
                                            callback());
    text(self_name, "b").set("c");
    EXPECT_EQ(**data_->text_store.getRecv(self_name, "b"), "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set("c"), std::invalid_argument);
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
    data_->value_change_event.appendListener(FieldBase{self_name, "d"},
                                             callback());
    value(self_name, "d").set({{"a", 100}});
    // dにはセットしてないのでeventは発動しない
    EXPECT_EQ(callback_called, 0);

    data_->value_change_event.appendListener(FieldBase{self_name, "d.a"},
                                             callback());
    value(self_name, "d")
        .set({{"a", 1},
              {"b", 2},
              {"c", {{"a", 1}, {"b", 2}}},
              {"v", {1, 2, 3, 4, 5}}});
    // d.aではevent発動する
    EXPECT_EQ(callback_called, 1);
    // 値がセットされている
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.a"), 1);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.b"), 2);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.c.a"), 1);
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.c.b"), 2);
    // 1つの値として取得した場合1つ目の要素
    EXPECT_EQ(**data_->value_store.getRecv(self_name, "d.v"), 1);
    EXPECT_EQ(static_cast<std::vector<double>>(
                  **data_->value_store.getRecv(self_name, "d.v"))
                  .size(),
              5);
}
TEST_F(DataTest, textSetDict) {
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

TEST_F(DataTest, valueGet) {
    data_->value_store.setRecv("a", "b",
                               std::make_shared<VectorOpt<double>>(123));
    EXPECT_EQ(value("a", "b").tryGet().value(), 123);
    EXPECT_EQ(value("a", "b").get(), 123);
    EXPECT_EQ(value("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(value("a", "c").get(), 0);
    EXPECT_EQ(data_->value_store.transferReq(true).at("a").at("b"), 1);
    EXPECT_EQ(data_->value_store.transferReq(true).at("a").at("c"), 2);
    EXPECT_EQ(value(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->value_store.transferReq(true).count(self_name), 0);
    value("a", "d").appendListener(callback<Value>());
    EXPECT_EQ(data_->value_store.transferReq(true).at("a").at("d"), 3);
}
TEST_F(DataTest, valueGetDict) {
    data_->value_store.setRecv("a", "d.a",
                               std::make_shared<VectorOpt<double>>(1));
    data_->value_store.setRecv("a", "d.b",
                               std::make_shared<VectorOpt<double>>(2));
    data_->value_store.setRecv("a", "d.c.a",
                               std::make_shared<VectorOpt<double>>(1));
    data_->value_store.setRecv("a", "d.c.b",
                               std::make_shared<VectorOpt<double>>(2));
    data_->value_store.setRecv("a", "d.v",
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
    data_->text_store.setRecv("a", "b", std::make_shared<std::string>("hoge"));
    EXPECT_EQ(text("a", "b").tryGet().value(), "hoge");
    EXPECT_EQ(text("a", "b").get(), "hoge");
    EXPECT_EQ(text("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "c").get(), "");
    EXPECT_EQ(data_->text_store.transferReq(true).at("a").at("b"), 1);
    EXPECT_EQ(data_->text_store.transferReq(true).at("a").at("c"), 2);
    EXPECT_EQ(text(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->text_store.transferReq(true).count(self_name), 0);
    text("a", "d").appendListener(callback<Text>());
    EXPECT_EQ(data_->text_store.transferReq(true).at("a").at("d"), 3);
}
TEST_F(DataTest, textGetDict) {
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

TEST_F(DataTest, logGet) {
    using namespace std::chrono;
    auto logs = std::make_shared<std::vector<std::shared_ptr<LogLine>>>(
        std::vector<std::shared_ptr<LogLine>>{
            std::make_shared<LogLine>(1, system_clock::now(), "a"),
            std::make_shared<LogLine>(2, system_clock::now(), "b"),
            std::make_shared<LogLine>(3, system_clock::now(), "c"),
        });
    data_->log_store.setRecv("a", logs);
    EXPECT_EQ(log("a").tryGet().value().size(), 3);
    EXPECT_EQ(log("a").get().size(), 3);
    EXPECT_EQ(log("b").tryGet(), std::nullopt);
    EXPECT_EQ(log("b").get().size(), 0);
    EXPECT_EQ(data_->log_store.transferReq(true).at("a"), true);
    EXPECT_EQ(data_->log_store.transferReq(true).at("b"), true);
    EXPECT_EQ(log(self_name).tryGet(), std::nullopt);
    EXPECT_EQ(data_->log_store.transferReq(true).count(self_name), 0);
    log("d").appendListener(callback<Log>());
    EXPECT_EQ(data_->log_store.transferReq(true).at("d"), true);
}
TEST_F(DataTest, logClear) {
    using namespace std::chrono;
    auto logs = std::make_shared<std::vector<std::shared_ptr<LogLine>>>(
        std::vector<std::shared_ptr<LogLine>>{
            std::make_shared<LogLine>(1, system_clock::now(), "a"),
            std::make_shared<LogLine>(2, system_clock::now(), "b"),
            std::make_shared<LogLine>(3, system_clock::now(), "c"),
        });
    data_->log_store.setRecv("a", logs);
    log("a").clear();
    EXPECT_EQ(log("a").tryGet().value().size(), 0);
}
TEST_F(DataTest, time) {
    auto t = std::chrono::system_clock::now();
    data_->sync_time_store.setRecv("a", t);
    EXPECT_EQ(value("a", "b").time(), t);
    EXPECT_EQ(text("a", "b").time(), t);
}
// todo: hidden, free
