#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/logger.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "../message/message.h"

using namespace webcface;
class LoggerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        data_->start();
    }
    std::u8string self_name = u8"test";
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
        EXPECT_EQ((**ls)[i].level, i);
        EXPECT_EQ((**ls)[i].message, Encoding::castToU8(std::to_string(i)));
    }
}
TEST_F(LoggerTest, loggerBuf) {
    LoggerBuf b(data_->logger);
    std::ostream os(&b);
    os << "a\nb" << std::endl;
    auto ls = data_->log_store->getRecv(self_name);
    ASSERT_EQ((*ls)->size(), 2);
    EXPECT_EQ((**ls)[0].level, 2);
    EXPECT_EQ((**ls)[0].message, u8"a");
    EXPECT_EQ((**ls)[1].level, 2);
    EXPECT_EQ((**ls)[1].message, u8"b");
}
