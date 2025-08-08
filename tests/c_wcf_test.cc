#include "test_common.h"
#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/func.h"
#include <gtest/gtest.h>
#include <webcface/member.h>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/wcf.h>
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/internal/component_internal.h"
#include <chrono>
#include <thread>
#include "dummy_server.h"

using namespace webcface;

class CClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dummy_s = std::make_shared<DummyServer>();
        wait();
        wcli_ = wcfInit(self_name.decode().data(), "127.0.0.1", 17530);
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
    while (!dummy_s->connected() || !wcfIsConnected(wcli_)) {
        wait();
    }

    EXPECT_EQ(wcfStart(nullptr), WCF_BAD_WCLI);
}
TEST_F(CClientTest, connectionBySync) {
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcfIsConnected(wcli_));
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    while (!dummy_s->connected() || !wcfIsConnected(wcli_)) {
        wait();
    }

    EXPECT_EQ(wcfSync(nullptr), WCF_BAD_WCLI);
}
TEST_F(CClientTest, connectionByWait) {
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcfIsConnected(wcli_));
    std::promise<void> p;
    auto f = p.get_future();
    std::thread t([&] {
        EXPECT_EQ(wcfWaitConnection(wcli_), WCF_OK);
        p.set_value();
    });
    while (!dummy_s->connected() || !wcfIsConnected(wcli_)) {
        wait();
    }
    dummy_s->waitRecv<message::SyncInit>([&](const auto &) {});
    EXPECT_NE(f.wait_for(std::chrono::milliseconds(0)),
              std::future_status::ready);
    dummy_s->send(message::SyncInitEnd{{}, "", "", 0, ""});
    t.join();
    f.get();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcfIsConnected(wcli_));

    EXPECT_EQ(wcfWaitConnection(nullptr), WCF_BAD_WCLI);
}

std::function<void(const char *, void *)> callback1_obj;
void callback1(const char *m, void *u) { callback1_obj(m, u); }
TEST_F(CClientTest, MemberList) {
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    while (!dummy_s->connected() || !wcfIsConnected(wcli_)) {
        wait();
    }
    int u_obj = 42;
    int called = 0;
    using namespace std::string_literals;
    callback1_obj = [&](const char *m, void *u) {
        EXPECT_EQ(m, "a"s);
        EXPECT_EQ(u, &u_obj);
        called++;
    };
    EXPECT_EQ(wcfMemberEntryEvent(wcli_, callback1, &u_obj), WCF_OK);
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "b", "1", "12345"});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
    EXPECT_EQ(called, 1);
    const char *members[3] = {};
    int member_num = 0;
    EXPECT_EQ(wcfMemberList(wcli_, members, sizeof(members), &member_num),
              WCF_OK);
    EXPECT_EQ(member_num, 1);
    ASSERT_NE(members[0], nullptr);
    EXPECT_EQ(members[0], "a"s);
    EXPECT_EQ(members[1], nullptr);
    EXPECT_EQ(members[2], nullptr);

    EXPECT_EQ(wcfMemberLibName(wcli_, "a"), "b"s);
    EXPECT_EQ(wcfMemberLibVersion(wcli_, "a"), "1"s);
    EXPECT_EQ(wcfMemberRemoteAddr(wcli_, "a"), "12345"s);
}
TEST_F(CClientTest, serverVersion) {
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    while (!dummy_s->connected() || !wcfIsConnected(wcli_)) {
        wait();
    }
    dummy_s->waitRecv<message::SyncInit>([&](const auto &) {});
    dummy_s->send(message::SyncInitEnd{{}, "a", "1", 0, "b"});
    wait();
    using namespace std::string_literals;
    EXPECT_EQ(wcfServerName(wcli_), ""s);
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
    EXPECT_EQ(wcfServerName(wcli_), "a"s);
    EXPECT_EQ(wcfServerVersion(wcli_), "1"s);
    EXPECT_EQ(wcfServerHostName(wcli_), "b"s);
}

TEST_F(CClientTest, valueSend) {
    EXPECT_EQ(wcfValueSet(wcli_, "a", 5), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    dummy_s->waitRecv<message::Value>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "a");
        EXPECT_EQ(obj.data.size(), 1u);
        EXPECT_EQ(obj.data.at(0), 5);
    });
    dummy_s->recvClear();

    double b[3] = {1, 1.5, 2};
    EXPECT_EQ(wcfValueSetVecD(wcli_, "b", b, 3), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    dummy_s->waitRecv<message::Value>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.data.size(), 3u);
        EXPECT_EQ(obj.data.at(0), 1);
        EXPECT_EQ(obj.data.at(1), 1.5);
        EXPECT_EQ(obj.data.at(2), 2);
    });
    dummy_s->recvClear();
}
TEST_F(CClientTest, valueReq) {
    double value[5] = {1, 1, 1, 1, 1};
    double value1 = 1;
    int size;
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, -1, &size),
              WCF_INVALID_ARGUMENT);
    EXPECT_EQ(wcfValueGetVecD(wcli_, "a", "b", value, 5, &size), WCF_NO_DATA);
    EXPECT_EQ(value[0], 0);
    EXPECT_EQ(value[1], 0);
    EXPECT_EQ(value[2], 0);
    EXPECT_EQ(value[3], 0);
    EXPECT_EQ(value[4], 0);
    EXPECT_EQ(wcfValueGet(wcli_, "a", "b", &value1), WCF_NO_DATA);
    EXPECT_EQ(value1, 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    dummy_s->waitRecv<message::Req<message::Value>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8StringView(), "a");
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    dummy_s->send(message::Res<message::Value>{
        1, ""_ss, MutableNumVector(std::vector<double>{1, 1.5, 2})});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
    dummy_s->send(message::Res<message::Value>{
        1, "c"_ss, MutableNumVector(std::vector<double>{1, 1.5, 2})});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
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
    dummy_s->waitRecv<message::Text>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "a");
        EXPECT_EQ(obj.data, "hello");
    });
    dummy_s->recvClear();

    EXPECT_EQ(wcfTextSetN(wcli_, "b", "hellohello", 5), WCF_OK);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    dummy_s->waitRecv<message::Text>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.data, "hello");
    });
    dummy_s->recvClear();
}
TEST_F(CClientTest, textReq) {
    char text[5] = {1, 1, 1, 1, 1};
    int size;
    EXPECT_EQ(wcfTextGet(wcli_, "a", "b", text, -1, &size),
              WCF_INVALID_ARGUMENT);
    EXPECT_EQ(wcfTextGet(wcli_, "a", "b", text, 5, &size), WCF_NO_DATA);
    EXPECT_EQ(text[0], 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    dummy_s->waitRecv<message::Req<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8StringView(), "a");
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    dummy_s->send(message::Res<message::Text>{1, ""_ss, ValAdaptor("hello")});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
    dummy_s->send(message::Res<message::Text>{1, "c"_ss, ValAdaptor("hello")});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
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
    wait();

    wcfMultiVal args[3] = {
        wcfValI(42),
        wcfValD(1.5),
        wcfValS("aaa"),
    };
    wcfMultiVal *ret, *async_ret;
    wcfPromise *async_res;
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
        dummy_s->waitRecv<message::Call>([&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, caller_id);
            EXPECT_EQ(obj.target_member_id, 0u);
            EXPECT_EQ(obj.field.u8StringView(), "b");
            EXPECT_EQ(obj.args.size(), 3u);
            EXPECT_EQ(static_cast<int>(obj.args.at(0)), 42);
            EXPECT_EQ(static_cast<double>(obj.args.at(1)), 1.5);
            EXPECT_EQ(static_cast<std::string_view>(obj.args.at(2)), "aaa");
        });
        dummy_s->recvClear();
        dummy_s->send(message::CallResponse{{}, caller_id, 1, false});
        EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
        caller_id++;
        // wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000);
    }

    // 2
    for (int i = 0; i < 2; i++) {
        dummy_s->waitRecv<message::Call>([&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, caller_id);
            EXPECT_EQ(obj.target_member_id, 0u);
            EXPECT_EQ(obj.field.u8StringView(), "b");
            EXPECT_EQ(obj.args.size(), 3u);
        });
        dummy_s->recvClear();
        dummy_s->send(message::CallResponse{{}, caller_id, 1, true});
        dummy_s->send(
            message::CallResult{{}, caller_id, 1, false, ValAdaptor("123.45")});
        EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
        caller_id++;
        // wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000);
    }

    // 3
    for (int i = 0; i < 2; i++) {
        dummy_s->waitRecv<message::Call>([&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, caller_id);
            EXPECT_EQ(obj.target_member_id, 0u);
            EXPECT_EQ(obj.field.u8StringView(), "b");
            EXPECT_EQ(obj.args.size(), 3u);
        });
        dummy_s->recvClear();
        dummy_s->send(message::CallResponse{{}, caller_id, 1, true});
        dummy_s->send(
            message::CallResult{{}, caller_id, 1, true, ValAdaptor("error")});
        EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
        caller_id++;
        // wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000);
    }

    t.join();
}

TEST_F(CClientTest, funcListen) {
    using namespace std::string_literals;
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);

    wcfValType arg_types[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli_, "a", arg_types, 3, WCF_VAL_INT);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    dummy_s->waitRecv<message::FuncInfo>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "a");
        EXPECT_EQ(obj.return_type, ValType::int_);
        ASSERT_EQ(obj.args.size(), 3u);
        EXPECT_EQ(obj.args.at(0)->type_, ValType::int_);
        EXPECT_EQ(obj.args.at(1)->type_, ValType::double_);
        EXPECT_EQ(obj.args.at(2)->type_, ValType::string_);
    });
    dummy_s->recvClear();

    wcfFuncCallHandle *h;
    EXPECT_EQ(wcfFuncFetchCall(wcli_, "a", &h), WCF_NOT_CALLED);
    EXPECT_EQ(wcfFuncFetchCall(wcli_, "b", &h), WCF_NOT_CALLED);
    dummy_s->send(message::Call{
        0, 1, 1, "a"_ss, {ValAdaptor(42), ValAdaptor(1.5), ValAdaptor("aaa")}});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);

    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.caller_member_id, 1u);
        EXPECT_TRUE(obj.started);
    });
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
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.caller_member_id, 1u);
        EXPECT_FALSE(obj.is_error);
        EXPECT_EQ(static_cast<double>(obj.result), 123.45);
    });
    dummy_s->recvClear();

    EXPECT_EQ(wcfFuncFetchCall(wcli_, "a", &h), WCF_NOT_CALLED);
}

std::function<void(wcfFuncCallHandle *h, void *u)> callback_obj = nullptr;
void callbackFunc(wcfFuncCallHandle *h, void *u) { callback_obj(h, u); }
TEST_F(CClientTest, funcSet) {
    using namespace std::string_literals;
    // EXPECT_EQ(wcfAutoSync(wcli_, 100), WCF_OK);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    auto main_id = std::this_thread::get_id();
    int u = 9999;
    wcfValType arg_types[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    callback_obj = [&](wcfFuncCallHandle *h, void *u) {
        EXPECT_EQ(*static_cast<int *>(u), 9999);
        EXPECT_EQ(std::this_thread::get_id(), main_id);

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
    dummy_s->waitRecv<message::FuncInfo>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "a");
        EXPECT_EQ(obj.return_type, ValType::int_);
        ASSERT_EQ(obj.args.size(), 3u);
        EXPECT_EQ(obj.args.at(0)->type_, ValType::int_);
        EXPECT_EQ(obj.args.at(1)->type_, ValType::double_);
        EXPECT_EQ(obj.args.at(2)->type_, ValType::string_);
    });
    dummy_s->recvClear();

    dummy_s->send(message::Call{
        0, 1, 1, "a"_ss, {ValAdaptor(42), ValAdaptor(1.5), ValAdaptor("aaa")}});
    wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000);

    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.caller_member_id, 1u);
        EXPECT_TRUE(obj.started);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.caller_member_id, 1u);
        EXPECT_FALSE(obj.is_error);
        EXPECT_EQ(static_cast<double>(obj.result), 123.45);
    });
}
TEST_F(CClientTest, funcSetAsync) {
    using namespace std::string_literals;
    // EXPECT_EQ(wcfAutoSync(wcli_, 100), WCF_OK);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    auto main_id = std::this_thread::get_id();
    int u = 9999;
    wcfValType arg_types[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    callback_obj = [&](wcfFuncCallHandle *h, void *u) {
        EXPECT_EQ(*static_cast<int *>(u), 9999);
        EXPECT_NE(std::this_thread::get_id(), main_id);

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
    wcfFuncSetAsync(wcli_, "a", arg_types, 3, WCF_VAL_INT, callbackFunc, &u);
    EXPECT_EQ(wcfSync(wcli_), WCF_OK);
    dummy_s->waitRecv<message::FuncInfo>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8StringView(), "a");
        EXPECT_EQ(obj.return_type, ValType::int_);
        ASSERT_EQ(obj.args.size(), 3u);
        EXPECT_EQ(obj.args.at(0)->type_, ValType::int_);
        EXPECT_EQ(obj.args.at(1)->type_, ValType::double_);
        EXPECT_EQ(obj.args.at(2)->type_, ValType::string_);
    });
    dummy_s->recvClear();

    dummy_s->send(message::Call{
        0, 1, 1, "a"_ss, {ValAdaptor(42), ValAdaptor(1.5), ValAdaptor("aaa")}});
    wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000);

    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.caller_member_id, 1u);
        EXPECT_TRUE(obj.started);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.caller_member_id, 1u);
        EXPECT_FALSE(obj.is_error);
        EXPECT_EQ(static_cast<double>(obj.result), 123.45);
    });
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
    dummy_s->waitRecv<message::View>([&](auto obj) {
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.data_ids->size(), 6u);
        EXPECT_EQ(obj.data_diff.size(), 6u);
        EXPECT_EQ(obj.data_diff["..0.0"]->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.data_diff["..0.0"]->text.u8StringView(), "abc");
        EXPECT_EQ(obj.data_diff["..1.0"]->type,
                  static_cast<int>(ViewComponentType::new_line));
        EXPECT_EQ(obj.data_diff["..0.1"]->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.data_diff["..0.1"]->text.u8StringView(), "123");

        EXPECT_EQ(obj.data_diff["..1.1"]->type,
                  static_cast<int>(ViewComponentType::new_line));

        EXPECT_EQ(obj.data_diff["..2.0"]->type,
                  static_cast<int>(ViewComponentType::button));
        EXPECT_EQ(obj.data_diff["..2.0"]->text.u8StringView(), "a");
        EXPECT_EQ(obj.data_diff["..2.0"]->on_click_member, self_name);
        EXPECT_EQ(obj.data_diff["..2.0"]->on_click_field->u8StringView(), "c");

        EXPECT_EQ(obj.data_diff["..2.1"]->type,
                  static_cast<int>(ViewComponentType::button));
        EXPECT_EQ(obj.data_diff["..2.1"]->text.u8StringView(), "a");
        EXPECT_EQ(obj.data_diff["..2.1"]->on_click_member->u8StringView(), "b");
        EXPECT_EQ(obj.data_diff["..2.1"]->on_click_field->u8StringView(), "c");
        EXPECT_EQ(obj.data_diff["..2.1"]->text_color,
                  static_cast<int>(ViewColor::red));
        EXPECT_EQ(obj.data_diff["..2.1"]->bg_color,
                  static_cast<int>(ViewColor::green));
    });
    dummy_s->recvClear();
}
TEST_F(CClientTest, viewReq) {
    using namespace std::string_literals;
    wcfViewComponent *vc;
    int size = 1;
    EXPECT_EQ(wcfViewGet(wcli_, "a", "b", &vc, &size), WCF_NO_DATA);
    EXPECT_EQ(size, 0);
    EXPECT_EQ(wcfStart(wcli_), WCF_OK);
    dummy_s->waitRecv<message::Req<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8StringView(), "a");
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });

    std::map<std::string, std::shared_ptr<message::ViewComponentData>,
             std::less<>>
        v{
            {"0", ViewComponents::text("a")
                      .textColor(ViewColor::yellow)
                      .bgColor(ViewColor::green)
                      .component_v.lockTmp({}, ""_ss)},
            {"1", ViewComponents::newLine().lockTmp({}, ""_ss)},
            {"2", ViewComponents::button("a", Func{Field{{}, "x"_ss, "y"_ss}})
                      .lockTmp({}, ""_ss)},
        };
    std::vector<SharedString> v_ids = {"0"_ss, "1"_ss, "2"_ss};
    dummy_s->send(message::Res<message::View>{1, ""_ss, v, v_ids});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
    dummy_s->send(message::Res<message::View>{1, "c"_ss, v, v_ids});
    EXPECT_EQ(wcfLoopSyncFor(wcli_, WEBCFACE_TEST_TIMEOUT * 1000), WCF_OK);
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
