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
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, -1, &size),
              WCF_INVALID_ARGUMENT);
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

TEST_F(CClientTest, funcRun) {
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);

    wcfMultiVal args[3] = {
        wcfValI(42),
        wcfValD(1.5),
        wcfValS("aaa"),
    };
    wcfMultiVal *ret;
    EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, -1, &ret),
              WCF_INVALID_ARGUMENT);
    std::thread t([&]() {
        // 1
        EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, 3, &ret), WCF_NOT_FOUND);
        // 2
        EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, 3, &ret), WCF_OK);
        EXPECT_EQ(ret->as_int, 123);
        EXPECT_EQ(ret->as_double, 123.45);
        EXPECT_EQ(std::string(ret->as_str), "123.45");
        // 3
        EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, 3, &ret), WCF_EXCEPTION);
        EXPECT_EQ(ret->as_int, 0);
        EXPECT_EQ(ret->as_double, 0);
        EXPECT_EQ(std::string(ret->as_str), "error");
    });

    std::size_t caller_id = 0;
    // 1
    wait();
    dummy_s->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, caller_id);
            EXPECT_EQ(obj.target_member_id, 0);
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.args.size(), 3);
            EXPECT_EQ(static_cast<int>(obj.args.at(0)), 42);
            EXPECT_EQ(static_cast<double>(obj.args.at(1)), 1.5);
            EXPECT_EQ(static_cast<std::string>(obj.args.at(2)), "aaa");
        },
        [&]() { ADD_FAILURE() << "Call recv error"; });
    dummy_s->recvClear();
    dummy_s->send(Message::CallResponse{{}, caller_id, 1, false});
    caller_id++;

    // 2
    wait();
    dummy_s->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, caller_id);
            EXPECT_EQ(obj.target_member_id, 0);
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.args.size(), 3);
        },
        [&]() { ADD_FAILURE() << "Call recv error"; });
    dummy_s->recvClear();
    dummy_s->send(Message::CallResponse{{}, caller_id, 1, true});
    dummy_s->send(Message::CallResult{{}, caller_id, 1, false, "123.45"});
    caller_id++;

    // 3
    wait();
    dummy_s->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, caller_id);
            EXPECT_EQ(obj.target_member_id, 0);
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.args.size(), 3);
        },
        [&]() { ADD_FAILURE() << "Call recv error"; });
    dummy_s->recvClear();
    dummy_s->send(Message::CallResponse{{}, caller_id, 1, true});
    dummy_s->send(Message::CallResult{{}, caller_id, 1, true, "error"});

    t.join();
}

TEST_F(CClientTest, funcListen) {
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);

    int arg_types[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli_, "a", arg_types, 3, WCF_VAL_INT);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.return_type, ValType::int_);
            EXPECT_EQ(obj.args->size(), 3);
            EXPECT_EQ(obj.args->at(0).type(), ValType::int_);
            EXPECT_EQ(obj.args->at(1).type(), ValType::double_);
            EXPECT_EQ(obj.args->at(2).type(), ValType::string_);
        },
        [&] { ADD_FAILURE() << "FuncInfo recv error"; });
    dummy_s->recvClear();

    wcfFuncCallHandle *h;
    EXPECT_EQ(wcfFuncFetchCall(wcli_, "a", &h), WCF_NOT_CALLED);
    EXPECT_EQ(wcfFuncFetchCall(wcli_, "b", &h), WCF_NOT_CALLED);
    dummy_s->send(Message::Call{{0, 1, 1, "a", {42, 1.5, "aaa"}}});
    wait();

    dummy_s->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 0);
            EXPECT_EQ(obj.caller_member_id, 1);
            EXPECT_TRUE(obj.started);
        },
        [&] { ADD_FAILURE() << "CallResponse recv error"; });
    dummy_s->recvClear();
    EXPECT_EQ(wcfFuncFetchCall(wcli_, "a", &h), WCF_OK);
    EXPECT_EQ(h->arg_size, 3);
    EXPECT_EQ(h->args[0].as_int, 42);
    EXPECT_EQ(h->args[0].as_double, 42.0);
    EXPECT_EQ(std::string(h->args[0].as_str), "42");
    EXPECT_EQ(h->args[1].as_double, 1.5);
    EXPECT_EQ(std::string(h->args[2].as_str), "aaa");

    wcfMultiVal ans = {.as_double = 123.45};
    EXPECT_EQ(wcfFuncRespond(h, &ans), WCF_OK);
    EXPECT_EQ(wcfFuncRespond(h, &ans), WCF_BAD_HANDLE);
    EXPECT_EQ(wcfFuncRespond(nullptr, &ans), WCF_BAD_HANDLE);
    wait();
    dummy_s->recv<Message::CallResult>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 0);
            EXPECT_EQ(obj.caller_member_id, 1);
            EXPECT_FALSE(obj.is_error);
            EXPECT_EQ(static_cast<double>(obj.result), 123.45);
        },
        [&] { ADD_FAILURE() << "CallResult recv error"; });
    dummy_s->recvClear();

    EXPECT_EQ(wcfFuncFetchCall(wcli_, "a", &h), WCF_NOT_CALLED);
}
