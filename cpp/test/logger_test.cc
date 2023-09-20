#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/logger.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "../message/message.h"

using namespace WebCFace;
class LoggerTest : public ::testing::Test {
  protected:
    void SetUp() override {
      data_ = std::make_shared<ClientData>(self_name);
    }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
};

TEST_F(LoggerTest, logger){
  data_->logger->trace("0");
  data_->logger->debug("1");
  data_->logger->info("2");
  data_->logger->warn("3");
  data_->logger->error("4");
  data_->logger->critical("5");
  for(int i = 0; i <= 5; i++){
    auto l = data_->logger_sink->pop();
    EXPECT_EQ((*l)->level, i);
    EXPECT_EQ((*l)->message, std::to_string(i));
  }
}
TEST_F(LoggerTest, loggerBuf){
  LoggerBuf b(data_);
  std::ostream os(&b);
  os << "a\nb" << std::endl;
  auto l = data_->logger_sink->pop();
  EXPECT_EQ((*l)->level, 2);
  EXPECT_EQ((*l)->message, "a");
  l = data_->logger_sink->pop();
  EXPECT_EQ((*l)->level, 2);
  EXPECT_EQ((*l)->message, "b");
}