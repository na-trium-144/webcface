#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include "webcface/internal/log_history.h"
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

class LogTest : public ::testing::Test {
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
    Log log(const SharedString &member, const SharedString &name) {
        return Log{Field{data_, member, name}};
    }
    Log log(std::string_view member, std::string_view name) {
        return Log{Field{data_, SharedString::fromU8String(member),
                         SharedString::fromU8String(name)}};
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

TEST_F(LogTest, field) {
    EXPECT_EQ(log("a", "b").member().name(), "a");
    EXPECT_THROW(Log().tryGet(), std::runtime_error);
}
TEST_F(LogTest, eventTarget) {
    log("a", "b").onChange(callback<Log>());
    data_->log_append_event["a"_ss]["b"_ss]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(LogTest, logGet) {
    using namespace std::chrono;
    auto logs = std::make_shared<webcface::internal::LogHistory>(
        std::deque<LogLineData>{
            {1, system_clock::now(), "a"_ss},
            {2, system_clock::now(), "b"_ss},
            {3, system_clock::now(), "c"_ss},
        });
    data_->log_store.setRecv("a"_ss, "b"_ss, logs);
    EXPECT_EQ(log("a", "b").tryGet().value().size(), 3u);
    EXPECT_EQ(log("a", "b").tryGetW().value().size(), 3u);
    ASSERT_EQ(log("a", "b").get().size(), 3u);
    ASSERT_EQ(log("a", "b").getW().size(), 3u);
    EXPECT_EQ(log("a", "b").get()[2].level(), 3);
    EXPECT_EQ(log("a", "b").getW()[2].level(), 3);
    EXPECT_EQ(log("a", "b").get()[2].message(), "c");
    EXPECT_EQ(log("a", "b").getW()[2].message(), L"c");
    EXPECT_EQ(log("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(log("a", "c").tryGetW(), std::nullopt);
    EXPECT_EQ(log("a", "c").get().size(), 0u);
    EXPECT_EQ(log("a", "c").getW().size(), 0u);
    EXPECT_EQ(data_->log_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->log_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_FALSE(log(self_name, "default"_ss).tryGet().has_value());
    EXPECT_FALSE(log(self_name, "default"_ss).tryGetW().has_value());
    EXPECT_FALSE(log(self_name, "a"_ss).tryGet().has_value());
    EXPECT_FALSE(log(self_name, "a"_ss).tryGetW().has_value());
    EXPECT_EQ(log(self_name, "a"_ss).get().size(), 0u);
    EXPECT_EQ(log(self_name, "a"_ss).getW().size(), 0u);
    EXPECT_EQ(data_->log_store.transferReq().count(self_name), 0u);
    log("a", "d").onChange(callback<Log>());
    EXPECT_EQ(data_->log_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}
TEST_F(LogTest, logClear) {
    using namespace std::chrono;
    auto logs = std::make_shared<webcface::internal::LogHistory>(
        std::deque<LogLineData>{
            {1, system_clock::now(), "a"_ss},
            {2, system_clock::now(), "b"_ss},
            {3, system_clock::now(), "c"_ss},
        });
    data_->log_store.setRecv("a"_ss, "b"_ss, logs);
    log("a", "b").clear();
    EXPECT_EQ(log("a", "b").tryGet().value().size(), 0u);
    EXPECT_EQ(log("a", "b").tryGetW().value().size(), 0u);
}
// todo: hidden, free
