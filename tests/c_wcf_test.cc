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
#include <webcface/wcf.h>
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
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(Encoding::castToU8(std::string_view(str, len)));
}

class CClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dummy_s = std::make_shared<DummyServer>();
        wait();
        wcli_ = wcfInit(self_name.decode().c_str(), "127.0.0.1", 17530);
    }
    void TearDown() override {
        wcfClose(wcli_);
        wait();
        dummy_s.reset();
    }
    SharedString self_name = "test"_ss;
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
            EXPECT_EQ(obj.field, "a"_ss);
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
            EXPECT_EQ(obj.field, "b"_ss);
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
    double value1 = 1;
    int size;
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, -1, &size),
              WCF_INVALID_ARGUMENT);
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, 5, &size), WCF_NOT_FOUND);
    EXPECT_EQ(value[0], 0);
    EXPECT_EQ(value[1], 0);
    EXPECT_EQ(value[2], 0);
    EXPECT_EQ(value[3], 0);
    EXPECT_EQ(value[4], 0);
    EXPECT_EQ(wcfValueGet(wcli_, "a", "b", &value1), WCF_NOT_FOUND);
    EXPECT_EQ(value1, 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Req<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a"_ss);
            EXPECT_EQ(obj.field, "b"_ss);
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Value Req recv error"; });
    dummy_s->send(Message::Res<Message::Value>{
        1, ""_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 1.5, 2})});
    dummy_s->send(Message::Res<Message::Value>{
        1, "c"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 1.5, 2})});
    wait();
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, 5, &size), WCF_OK);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(value[0], 1);
    EXPECT_EQ(value[1], 1.5);
    EXPECT_EQ(value[2], 2);
    EXPECT_EQ(value[3], 0);
    EXPECT_EQ(value[4], 0);
    EXPECT_EQ(wcfValueGet(wcli_, "a", "b", &value1), WCF_OK);
    EXPECT_EQ(value1, 1);

    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    value1 = 0;
    size = 0;
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b.c", value, 5, &size), WCF_OK);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(wcfValueGet(wcli_, "a", "b.c", &value1), WCF_OK);
    EXPECT_EQ(value1, 1);
}
TEST_F(CClientTest, textSend) {
    EXPECT_EQ(wcfTextSet(wcli_, "a", "hello"), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a"_ss);
            EXPECT_EQ(*obj.data, "hello");
        },
        [&] { ADD_FAILURE() << "Text recv error"; });
    dummy_s->recvClear();

    EXPECT_EQ(wcfTextSetN(wcli_, "b", "hellohello", 5), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "b"_ss);
            EXPECT_EQ(*obj.data, "hello");
        },
        [&] { ADD_FAILURE() << "Text recv error"; });
    dummy_s->recvClear();
}
TEST_F(CClientTest, textReq) {
    char text[5] = {1, 1, 1, 1, 1};
    int size;
    EXPECT_EQ(wcfTextGet(wcli_, "a", "b", text, -1, &size),
              WCF_INVALID_ARGUMENT);
    EXPECT_EQ(wcfTextGet(wcli_, "a", "b", text, 5, &size), WCF_NOT_FOUND);
    EXPECT_EQ(text[0], 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Req<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a"_ss);
            EXPECT_EQ(obj.field, "b"_ss);
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Text Req recv error"; });
    dummy_s->send(Message::Res<Message::Text>{
        1, ""_ss, std::make_shared<ValAdaptor>("hello")});
    dummy_s->send(Message::Res<Message::Text>{
        1, "c"_ss, std::make_shared<ValAdaptor>("hello")});
    wait();
    EXPECT_EQ(wcfTextGet(wcli_, "a", "b", text, 5, &size), WCF_OK);
    EXPECT_EQ(size, 5);
    EXPECT_EQ(text[0], 'h');
    EXPECT_EQ(text[1], 'e');
    EXPECT_EQ(text[2], 'l');
    EXPECT_EQ(text[3], 'l');
    EXPECT_EQ(text[4], 0);

    size = 0;
    EXPECT_EQ(wcfTextGet(wcli_, "a", "b.c", text, 5, &size), WCF_OK);
    EXPECT_EQ(size, 5);
}

TEST_F(CClientTest, funcRun) {
    using namespace std::string_literals;
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);

    wcfMultiVal args[3] = {
        wcfValI(42),
        wcfValD(1.5),
        wcfValS("aaa"),
    };
    wcfMultiVal *ret, *async_ret;
    wcfAsyncFuncResult *async_res;
    EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, -1, &ret),
              WCF_INVALID_ARGUMENT);
    EXPECT_EQ(wcfFuncRunAsync(wcli_, "a", "b", args, -1, &async_res),
              WCF_INVALID_ARGUMENT);
    std::thread t([&]() {
        // 1
        EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, 3, &ret), WCF_NOT_FOUND);
        wcfFuncRunAsync(wcli_, "a", "b", args, 3, &async_res);
        EXPECT_EQ(wcfFuncGetResult(async_res, &async_ret), WCF_NOT_RETURNED);
        EXPECT_EQ(wcfDestroy(async_ret), WCF_BAD_HANDLE);
        EXPECT_EQ(wcfFuncWaitResult(async_res, &async_ret), WCF_NOT_FOUND);
        EXPECT_EQ(wcfDestroy(async_ret), WCF_OK);
        EXPECT_EQ(wcfFuncWaitResult(async_res, &async_ret), WCF_BAD_HANDLE);
        // 2
        EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, 3, &ret), WCF_OK);
        EXPECT_EQ(ret->as_int, 123);
        EXPECT_EQ(ret->as_double, 123.45);
        EXPECT_EQ(ret->as_str, "123.45"s);
        EXPECT_EQ(wcfDestroy(ret), WCF_OK);
        EXPECT_EQ(wcfDestroy(ret), WCF_BAD_HANDLE);
        wcfFuncRunAsync(wcli_, "a", "b", args, 3, &async_res);
        EXPECT_EQ(wcfFuncWaitResult(async_res, &async_ret), WCF_OK);
        EXPECT_EQ(async_ret->as_int, 123);
        EXPECT_EQ(async_ret->as_double, 123.45);
        EXPECT_EQ(async_ret->as_str, "123.45"s);
        EXPECT_EQ(wcfDestroy(async_ret), WCF_OK);
        EXPECT_EQ(wcfDestroy(async_ret), WCF_BAD_HANDLE);
        // 3
        EXPECT_EQ(wcfFuncRun(wcli_, "a", "b", args, 3, &ret), WCF_EXCEPTION);
        EXPECT_EQ(ret->as_int, 0);
        EXPECT_EQ(ret->as_double, 0);
        EXPECT_EQ(ret->as_str, "error"s);
        EXPECT_EQ(wcfDestroy(ret), WCF_OK);
        wcfFuncRunAsync(wcli_, "a", "b", args, 3, &async_res);
        EXPECT_EQ(wcfFuncWaitResult(async_res, &async_ret), WCF_EXCEPTION);
        EXPECT_EQ(async_ret->as_int, 0);
        EXPECT_EQ(async_ret->as_double, 0);
        EXPECT_EQ(async_ret->as_str, "error"s);
        EXPECT_EQ(wcfDestroy(async_ret), WCF_OK);
    });

    std::size_t caller_id = 0;
    // 1
    for (int i = 0; i < 2; i++) {
        wait();
        dummy_s->recv<Message::Call>(
            [&](const auto &obj) {
                EXPECT_EQ(obj.caller_id, caller_id);
                EXPECT_EQ(obj.target_member_id, 0);
                EXPECT_EQ(obj.field, "b"_ss);
                EXPECT_EQ(obj.args.size(), 3);
                EXPECT_EQ(static_cast<int>(obj.args.at(0)), 42);
                EXPECT_EQ(static_cast<double>(obj.args.at(1)), 1.5);
                EXPECT_EQ(static_cast<std::string>(obj.args.at(2)), "aaa");
            },
            [&]() { ADD_FAILURE() << "Call recv error"; });
        dummy_s->recvClear();
        dummy_s->send(Message::CallResponse{{}, caller_id, 1, false});
        caller_id++;
    }

    // 2
    for (int i = 0; i < 2; i++) {
        wait();
        dummy_s->recv<Message::Call>(
            [&](const auto &obj) {
                EXPECT_EQ(obj.caller_id, caller_id);
                EXPECT_EQ(obj.target_member_id, 0);
                EXPECT_EQ(obj.field, "b"_ss);
                EXPECT_EQ(obj.args.size(), 3);
            },
            [&]() { ADD_FAILURE() << "Call recv error"; });
        dummy_s->recvClear();
        dummy_s->send(Message::CallResponse{{}, caller_id, 1, true});
        dummy_s->send(
            Message::CallResult{{}, caller_id, 1, false, ValAdaptor("123.45")});
        caller_id++;
    }

    // 3
    for (int i = 0; i < 2; i++) {
        wait();
        dummy_s->recv<Message::Call>(
            [&](const auto &obj) {
                EXPECT_EQ(obj.caller_id, caller_id);
                EXPECT_EQ(obj.target_member_id, 0);
                EXPECT_EQ(obj.field, "b"_ss);
                EXPECT_EQ(obj.args.size(), 3);
            },
            [&]() { ADD_FAILURE() << "Call recv error"; });
        dummy_s->recvClear();
        dummy_s->send(Message::CallResponse{{}, caller_id, 1, true});
        dummy_s->send(
            Message::CallResult{{}, caller_id, 1, true, ValAdaptor("error")});
        caller_id++;
    }

    t.join();
}

TEST_F(CClientTest, funcListen) {
    using namespace std::string_literals;
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);

    int arg_types[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli_, "a", arg_types, 3, WCF_VAL_INT);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a"_ss);
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
    dummy_s->send(
        Message::Call{{0,
                       1,
                       1,
                       "a"_ss,
                       {ValAdaptor(42), ValAdaptor(1.5), ValAdaptor("aaa")}}});
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
    EXPECT_EQ(h->args[0].as_str, "42"s);
    EXPECT_EQ(h->args[1].as_double, 1.5);
    EXPECT_EQ(h->args[2].as_str, "aaa"s);

    wcfMultiVal ans = wcfValD(123.45);
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

std::function<void(wcfFuncCallHandle *h, void *u)> callback_obj = nullptr;
void callbackFunc(wcfFuncCallHandle *h, void *u) { callback_obj(h, u); }
TEST_F(CClientTest, funcSet) {
    using namespace std::string_literals;
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    int u = 9999;
    int arg_types[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    callback_obj = [&](wcfFuncCallHandle *h, void *u) {
        EXPECT_EQ(*static_cast<int *>(u), 9999);

        EXPECT_EQ(h->arg_size, 3);
        EXPECT_EQ(h->args[0].as_int, 42);
        EXPECT_EQ(h->args[0].as_double, 42.0);
        EXPECT_EQ(h->args[0].as_str, "42"s);
        EXPECT_EQ(h->args[1].as_double, 1.5);
        EXPECT_EQ(h->args[2].as_str, "aaa"s);

        wcfMultiVal ans = wcfValD(123.45);
        EXPECT_EQ(wcfFuncRespond(h, &ans), WCF_OK);
        EXPECT_EQ(wcfFuncRespond(h, &ans), WCF_BAD_HANDLE);
        EXPECT_EQ(wcfFuncRespond(nullptr, &ans), WCF_BAD_HANDLE);
    };
    wcfFuncSet(wcli_, "a", arg_types, 3, WCF_VAL_INT, callbackFunc, &u);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a"_ss);
            EXPECT_EQ(obj.return_type, ValType::int_);
            EXPECT_EQ(obj.args->size(), 3);
            EXPECT_EQ(obj.args->at(0).type(), ValType::int_);
            EXPECT_EQ(obj.args->at(1).type(), ValType::double_);
            EXPECT_EQ(obj.args->at(2).type(), ValType::string_);
        },
        [&] { ADD_FAILURE() << "FuncInfo recv error"; });
    dummy_s->recvClear();

    dummy_s->send(
        Message::Call{{0,
                       1,
                       1,
                       "a"_ss,
                       {ValAdaptor(42), ValAdaptor(1.5), ValAdaptor("aaa")}}});
    wait();

    dummy_s->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 0);
            EXPECT_EQ(obj.caller_member_id, 1);
            EXPECT_TRUE(obj.started);
        },
        [&] { ADD_FAILURE() << "CallResponse recv error"; });
    dummy_s->recv<Message::CallResult>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 0);
            EXPECT_EQ(obj.caller_member_id, 1);
            EXPECT_FALSE(obj.is_error);
            EXPECT_EQ(static_cast<double>(obj.result), 123.45);
        },
        [&] { ADD_FAILURE() << "CallResult recv error"; });
}

TEST_F(CClientTest, viewSend) {
    wcfViewComponent vc[10];
    vc[0] = wcfText("abc\n123");
    vc[1] = wcfNewLine();
    vc[2] = wcfButton("a", nullptr, "c");
    vc[3] = wcfButton("a", "b", "c");
    vc[3].text_color = WCF_COLOR_RED;
    vc[3].bg_color = WCF_COLOR_GREEN;

    EXPECT_EQ(wcfViewSet(wcli_, "b", vc, 4), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::View>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "b"_ss);
            EXPECT_EQ(obj.length, 6);
            EXPECT_EQ(obj.data_diff->size(), 6);
            EXPECT_EQ((*obj.data_diff)["0"].type, ViewComponentType::text);
            EXPECT_EQ((*obj.data_diff)["0"].text, "abc"_ss);
            EXPECT_EQ((*obj.data_diff)["1"].type, ViewComponentType::new_line);
            EXPECT_EQ((*obj.data_diff)["2"].type, ViewComponentType::text);
            EXPECT_EQ((*obj.data_diff)["2"].text, "123"_ss);

            EXPECT_EQ((*obj.data_diff)["3"].type, ViewComponentType::new_line);

            EXPECT_EQ((*obj.data_diff)["4"].type, ViewComponentType::button);
            EXPECT_EQ((*obj.data_diff)["4"].text, "a"_ss);
            EXPECT_EQ((*obj.data_diff)["4"].on_click_member, self_name);
            EXPECT_EQ((*obj.data_diff)["4"].on_click_field, "c"_ss);

            EXPECT_EQ((*obj.data_diff)["5"].type, ViewComponentType::button);
            EXPECT_EQ((*obj.data_diff)["5"].text, "a"_ss);
            EXPECT_EQ((*obj.data_diff)["5"].on_click_member, "b"_ss);
            EXPECT_EQ((*obj.data_diff)["5"].on_click_field, "c"_ss);
            EXPECT_EQ((*obj.data_diff)["5"].text_color, ViewColor::red);
            EXPECT_EQ((*obj.data_diff)["5"].bg_color, ViewColor::green);
        },
        [&] { ADD_FAILURE() << "View recv error"; });
    dummy_s->recvClear();
}
TEST_F(CClientTest, viewReq) {
    using namespace std::string_literals;
    wcfViewComponent *vc;
    int size = 1;
    EXPECT_EQ(wcfViewGet(wcli_, "a", "b", &vc, &size), WCF_NOT_FOUND);
    EXPECT_EQ(size, 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    wait();
    dummy_s->recv<Message::Req<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a"_ss);
            EXPECT_EQ(obj.field, "b"_ss);
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "View Req recv error"; });

    auto v = std::make_shared<
        std::unordered_map<std::string, Message::View::ViewComponent>>(
        std::unordered_map<std::string, Message::View::ViewComponent>{
            {"0", ViewComponents::text("a")
                      .textColor(ViewColor::yellow)
                      .bgColor(ViewColor::green)
                      .toV()
                      .lockTmp({}, ""_ss)},
            {"1", ViewComponents::newLine().lockTmp({}, ""_ss)},
            {"2", ViewComponents::button("a", Func{Field{{}, "x"_ss, "y"_ss}})
                      .lockTmp({}, ""_ss)},
        });
    dummy_s->send(Message::Res<Message::View>{1, ""_ss, v, 3});
    dummy_s->send(Message::Res<Message::View>{1, "c"_ss, v, 3});
    wait();
    EXPECT_EQ(wcfViewGet(wcli_, "a", "b", &vc, &size), WCF_OK);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(vc[0].type, WCF_VIEW_TEXT);
    EXPECT_EQ(vc[0].text, "a"s);
    EXPECT_EQ(vc[0].on_click_member, nullptr);
    EXPECT_EQ(vc[0].on_click_field, nullptr);
    EXPECT_EQ(vc[0].text_color, WCF_COLOR_YELLOW);
    EXPECT_EQ(vc[0].bg_color, WCF_COLOR_GREEN);
    EXPECT_EQ(vc[1].type, WCF_VIEW_NEW_LINE);
    EXPECT_EQ(vc[2].type, WCF_VIEW_BUTTON);
    EXPECT_EQ(vc[2].text, "a"s);
    EXPECT_EQ(vc[2].on_click_member, "x"s);
    EXPECT_EQ(vc[2].on_click_field, "y"s);
    EXPECT_EQ(wcfDestroy(vc), WCF_OK);
    EXPECT_EQ(wcfDestroy(vc), WCF_BAD_HANDLE);

    size = 0;
    EXPECT_EQ(wcfViewGet(wcli_, "a", "b.c", &vc, &size), WCF_OK);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(wcfDestroy(vc), WCF_OK);
}
