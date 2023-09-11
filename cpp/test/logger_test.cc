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
  data->logger_.trace("0");
  data->logger_.debug("1");
  data->logger_.info("2");
  data->logger_.warn("3");
  data->logger_.error("4");
  data->logger_.critical("5");
  for(int i = 0; i <= 5; i++){
    auto l = data->logger_sink->pop();
    EXPECT_EQ(l->level, i);
    EXPECT_EQ(l->message, std::to_string(i));
  }
}
TEST_F(LoggerTest, loggerBuf){
  LoggerBuf b(data_);
  std::ostream os(&b);
  os << "a\nb" << std::endl;
  auto l = data->logger_sink->pop();
  EXPECT_EQ(l->level, 2);
  EXPECT_EQ(l->message, "a");
  l = data->logger_sink->pop();
  EXPECT_EQ(l->level, 2);
  EXPECT_EQ(l->message, "b");
}
