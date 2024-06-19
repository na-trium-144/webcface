#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/logger.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "../message/message.h"

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(Encoding::castToU8(std::string_view(str, len)));
}

class LoggerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        data_->start();
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<Internal::ClientData> data_;
};

TEST_F(LoggerTest, logger) {
    data_->logger->trace("0");
    data_->logger->debug("1");
    data_->logger->info("2");
    data_->logger->warn("3");
    data_->logger->error("4");
    data_->logger->critical("5");
    auto ls = data_->log_store->getRecv(self_name);
    ASSERT_EQ((*ls)->size(), 6);
    for (int i = 0; i <= 5; i++) {
        EXPECT_EQ((**ls)[i].level(), i);
        EXPECT_EQ((**ls)[i].message(), Encoding::castToU8(std::to_string(i)));
    }
}
TEST_F(LoggerTest, loggerBuf) {
    LoggerBuf b(data_->log_store);
    std::ostream os(&b);
    os << "a\nb" << std::endl;
    auto cerr_buf = std::cerr.rdbuf();
    std::cerr.rdbuf(&b);
    std::cerr << "c" << std::endl;
    auto ls = data_->log_store->getRecv(self_name);
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
    LoggerBufW b(data_->log_store);
    std::wostream os(&b);
    os << L"a\nb" << std::endl;
    auto wcerr_buf = std::wcerr.rdbuf();
    std::wcerr.rdbuf(&b);
    std::wcerr << L"c" << std::endl;
    auto ls = data_->log_store->getRecv(self_name);
    ASSERT_EQ((*ls)->size(), 3);
    EXPECT_EQ((**ls)[0].level(), 2);
    EXPECT_EQ((**ls)[0].message(), u8"a");
    EXPECT_EQ((**ls)[1].level(), 2);
    EXPECT_EQ((**ls)[1].message(), u8"b");
    EXPECT_EQ((**ls)[2].level(), 2);
    EXPECT_EQ((**ls)[2].message(), u8"c");
    std::wcerr.rdbuf(wcerr_buf);
}
