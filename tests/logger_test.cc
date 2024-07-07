#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/internal/logger.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "webcface/message/message.h"

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(encoding::castToU8(std::string_view(str, len)));
}

class LoggerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
        data_->start();
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
};

TEST_F(LoggerTest, loggerBuf) {
    LoggerBuf b(data_.get());
    std::ostream os(&b);
    os << "a\nb" << std::endl;
    auto cerr_buf = std::cerr.rdbuf();
    std::cerr.rdbuf(&b);
    std::cerr << "c" << std::endl;
    auto ls = data_->log_store.getRecv(self_name);
    ASSERT_EQ((*ls)->size(), 3);
    EXPECT_EQ((**ls)[0].level(), 2);
    EXPECT_EQ((**ls)[0].message(), u8"a");
    EXPECT_EQ((**ls)[1].level(), 2);
    EXPECT_EQ((**ls)[1].message(), u8"b");
    EXPECT_EQ((**ls)[2].level(), 2);
    EXPECT_EQ((**ls)[2].message(), u8"c");
    std::cerr.rdbuf(cerr_buf);
}
TEST_F(LoggerTest, loggerBufW) {
    LoggerBufW b(data_.get());
    std::wostream os(&b);
    os << L"a\nb" << std::endl;
    auto wcerr_buf = std::wcerr.rdbuf();
    std::wcerr.rdbuf(&b);
    std::wcerr << L"c" << std::endl;
    auto ls = data_->log_store.getRecv(self_name);
    ASSERT_EQ((*ls)->size(), 3);
    EXPECT_EQ((**ls)[0].level(), 2);
    EXPECT_EQ((**ls)[0].message(), u8"a");
    EXPECT_EQ((**ls)[1].level(), 2);
    EXPECT_EQ((**ls)[1].message(), u8"b");
    EXPECT_EQ((**ls)[2].level(), 2);
    EXPECT_EQ((**ls)[2].message(), u8"c");
    std::wcerr.rdbuf(wcerr_buf);
}
