#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/client.h>
#include <webcface/logger.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/common/def.h>
#include "../message/message.h"
#include <chrono>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace webcface;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}

class CClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dummy_s = std::make_shared<DummyServer>();
        wait();
        wcli_ = wcfInit(self_name.c_str(), "127.0.0.1", 17530);
    }
    void TearDown() override {
        wcfClose(wcli_);
        wait();
        dummy_s.reset();
    }
    std::string self_name = "test";
    wcfClient *wcli_;
    std::shared_ptr<DummyServer> dummy_s;
};

TEST_F(CClientTest, isValid) {
    EXPECT_TRUE(wcfIsValid(wcli_));
    EXPECT_FALSE(wcfIsValid(nullptr));
    EXPECT_FALSE(wcfIsValid(this));
}
TEST_F(CClientTest, connectionByStart) {
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcfIsConnected(wcli_));
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    wait();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcfIsConnected(wcli_));

    EXPECT_EQ(wcfStart(nullptr), WCF_BAD_WCLI);
}
TEST_F(CClientTest, connectionBySync) {
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcfIsConnected(wcli_));
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcfIsConnected(wcli_));

    EXPECT_EQ(wcfSync(nullptr), WCF_BAD_WCLI);
}
TEST_F(CClientTest, valueSend) {
    EXPECT_EQ(wcfValueSet(wcli_, "a", 5), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Value>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.data->size(), 1);
            EXPECT_EQ(obj.data->at(0), 5);
        },
        [&] { ADD_FAILURE() << "Value recv error"; });
    dummy_s->recvClear();

    double b[3] = {1, 1.5, 2};
    EXPECT_EQ(wcfValueSetVecD(wcli_, "b", b, 3), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Value>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.data->size(), 3);
            EXPECT_EQ(obj.data->at(0), 1);
            EXPECT_EQ(obj.data->at(1), 1.5);
            EXPECT_EQ(obj.data->at(2), 2);
        },
        [&] { ADD_FAILURE() << "Value recv error"; });
    dummy_s->recvClear();
}
TEST_F(CClientTest, valueReq) {
    double value[5] = {1, 1, 1, 1, 1};
    int size;
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, -1, &size), WCF_INVALID_ARGUMENT);
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, 5, &size), WCF_NOT_FOUND);
    EXPECT_EQ(value[0], 0);
    EXPECT_EQ(value[1], 0);
    EXPECT_EQ(value[2], 0);
    EXPECT_EQ(value[3], 0);
    EXPECT_EQ(value[4], 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Req<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Value Req recv error"; });
    dummy_s->send(Message::Res<Message::Value>{
        1, "",
        std::make_shared<std::vector<double>>(std::vector<double>{1, 1.5, 2})});
    dummy_s->send(Message::Res<Message::Value>{
        1, "c",
        std::make_shared<std::vector<double>>(std::vector<double>{1, 1.5, 2})});
    wait();
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, 5, &size), WCF_OK);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(value[0], 1);
    EXPECT_EQ(value[1], 1.5);
    EXPECT_EQ(value[2], 2);
    EXPECT_EQ(value[3], 0);
    EXPECT_EQ(value[4], 0);

    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    size = 0;
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b.c", value, 5, &size), WCF_OK);
    EXPECT_EQ(size, 3);
}
