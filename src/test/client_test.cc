#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/client.h>
#include <webcface/logger.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/common/def.h>
#include "../message/message.h"
#include <chrono>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace WebCFace;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}

class ClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        dummy_s = std::make_shared<DummyServer>();
        wait();
        WebCFace::logger_internal_level = spdlog::level::trace;
        data_ = std::make_shared<ClientData>(self_name);
        wcli_ = std::make_shared<Client>(self_name, "127.0.0.1", 17530, data_);
        callback_called = 0;
        dummy_s->recvClear();
        // 接続を待機する (todo: 接続完了まで待機する関数があると良い)
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown begin" << std::endl;
        wcli_.reset();
        data_.reset();
        wait();
        dummy_s.reset();
        std::cout << "TearDown end" << std::endl;
    }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
    std::shared_ptr<Client> wcli_;
    std::shared_ptr<DummyServer> dummy_s;
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(ClientTest, connection) {
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());
}
TEST_F(ClientTest, name) { EXPECT_EQ(wcli_->name(), self_name); }
TEST_F(ClientTest, memoryLeak) {
    wcli_.reset();
    wait();
    EXPECT_EQ(data_.use_count(), 1);
}
TEST_F(ClientTest, sync) {
    wcli_->sync();
    wait();
    using namespace WebCFace::Message;
    dummy_s->recv<SyncInit>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_name, self_name);
            EXPECT_EQ(obj.lib_name, "cpp");
            EXPECT_EQ(obj.lib_ver, WEBCFACE_VERSION);
        },
        [&] { ADD_FAILURE() << "SyncInit recv error"; });
    dummy_s->recv<Sync>([&](const auto &) {},
                        [&] { ADD_FAILURE() << "Sync recv error"; });
    // todo: 時刻が正しく変換できてるかテストする(めんどくさい)

    dummy_s->recvClear();
    wcli_->sync();
    wait();
    dummy_s->recv<SyncInit>(
        [&](const auto &) {
            ADD_FAILURE() << "should not send SyncInit twice";
        },
        [&] {});
    dummy_s->recv<Sync>([&](const auto &) {},
                        [&] { ADD_FAILURE() << "Sync recv error"; });
}
TEST_F(ClientTest, serverVersion) {
    dummy_s->send(Message::SvrVersion{{}, "a", "1"});
    wait();
    EXPECT_EQ(wcli_->serverName(), "a");
    EXPECT_EQ(wcli_->serverVersion(), "1");
}
TEST_F(ClientTest, ping) {
    dummy_s->send(Message::Ping{});
    wait();
    dummy_s->recv<Message::Ping>([&](const auto &) {},
                                 [&] { ADD_FAILURE() << "Ping recv error"; });

    wcli_->member("a").onPing().appendListener(callback<Member>());
    dummy_s->send(Message::SyncInit{{}, "a", 10, "", "", ""});
    dummy_s->send(Message::PingStatus{
        {},
        std::make_shared<std::unordered_map<unsigned int, int>>(
            std::unordered_map<unsigned int, int>{{10, 15}})});
    wait();
    dummy_s->recv<Message::PingStatusReq>(
        [&](const auto &) {},
        [&] { ADD_FAILURE() << "Ping Status Req recv error"; });
    EXPECT_EQ(callback_called, 1);
    EXPECT_EQ(wcli_->member("a").pingStatus().value(), 15);
}
TEST_F(ClientTest, entry) {
    wcli_->onMemberEntry().appendListener(callback<Member>());
    dummy_s->send(Message::SyncInit{{}, "a", 10, "b", "1", "12345"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(wcli_->members().size(), 1);
    EXPECT_EQ(wcli_->members()[0].name(), "a");
    EXPECT_EQ(data_->member_ids["a"], 10);

    auto m = wcli_->member("a");
    EXPECT_EQ(m.libName(), "b");
    EXPECT_EQ(m.libVersion(), "1");
    EXPECT_EQ(m.remoteAddr(), "12345");

    m.onValueEntry().appendListener(callback<Value>());
    dummy_s->send(Message::Entry<Message::Value>{{}, 10, "b"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(m.values().size(), 1);
    EXPECT_EQ(m.values()[0].name(), "b");

    m.onTextEntry().appendListener(callback<Text>());
    dummy_s->send(Message::Entry<Message::Text>{{}, 10, "c"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(m.texts().size(), 1);
    EXPECT_EQ(m.texts()[0].name(), "c");

    m.onViewEntry().appendListener(callback<View>());
    dummy_s->send(Message::Entry<Message::View>{{}, 10, "d"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(m.views().size(), 1);
    EXPECT_EQ(m.views()[0].name(), "d");

    m.onFuncEntry().appendListener(callback<Func>());
    dummy_s->send(Message::FuncInfo{
        10, "a", ValType::int_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>(1)});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(m.funcs().size(), 1);
    EXPECT_EQ(m.funcs()[0].name(), "a");

    m.onSync().appendListener(callback<Member>());
    dummy_s->send(Message::Sync{10, std::chrono::system_clock::now()});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ClientTest, valueSend) {
    data_->value_store.setSend("a", std::make_shared<VectorOpt<double>>(5));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Value>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.data->size(), 1);
            EXPECT_EQ(obj.data->at(0), 5);
        },
        [&] { ADD_FAILURE() << "Value recv error"; });
}
TEST_F(ClientTest, valueReq) {
    data_->value_store.getRecv("a", "b");
    wcli_->sync();
    wait();
    wcli_->member("a").value("b").appendListener(callback<Value>());
    dummy_s->recv<Message::Req<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Value Req recv error"; });
    dummy_s->send(Message::Res<Message::Value>{
        1, "",
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    dummy_s->send(Message::Res<Message::Value>{
        1, "c",
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->value_store.getRecv("a", "b").has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a", "b").value())
                  .size(),
              3);
    EXPECT_TRUE(data_->value_store.getRecv("a", "b.c").has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a", "b.c").value())
                  .size(),
              3);
}
TEST_F(ClientTest, textSend) {
    data_->text_store.setSend("a", std::make_shared<std::string>("b"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(*obj.data, "b");
        },
        [&] { ADD_FAILURE() << "Text recv error"; });
}
TEST_F(ClientTest, textReq) {
    data_->text_store.getRecv("a", "b");
    wcli_->sync();
    wait();
    wcli_->member("a").text("b").appendListener(callback<Text>());
    dummy_s->recv<Message::Req<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Text Req recv error"; });
    dummy_s->send(
        Message::Res<Message::Text>{1, "", std::make_shared<std::string>("z")});
    dummy_s->send(Message::Res<Message::Text>{
        1, "c", std::make_shared<std::string>("z")});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->text_store.getRecv("a", "b").has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a", "b").value(), "z");
    EXPECT_TRUE(data_->text_store.getRecv("a", "b.c").has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a", "b.c").value(), "z");
}
TEST_F(ClientTest, viewSend) {
    data_->view_store.setSend(
        "a", std::make_shared<std::vector<ViewComponentBase>>(
                 std::vector<ViewComponentBase>{
                     ViewComponents::text("a")
                         .textColor(ViewColor::yellow)
                         .bgColor(ViewColor::green)
                         .lockTmp(data_, ""),
                     ViewComponents::newLine().lockTmp(data_, ""),
                     ViewComponents::button("a", Func{Field{data_, "x", "y"}})
                         .lockTmp(data_, ""),
                 }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::View>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ((*obj.data_diff)["0"].type, ViewComponentType::text);
            EXPECT_EQ((*obj.data_diff)["0"].text, "a");
            EXPECT_EQ((*obj.data_diff)["0"].text_color, ViewColor::yellow);
            EXPECT_EQ((*obj.data_diff)["0"].bg_color, ViewColor::green);
            EXPECT_EQ((*obj.data_diff)["1"].type, ViewComponentType::new_line);
            EXPECT_EQ((*obj.data_diff)["2"].type, ViewComponentType::button);
            EXPECT_EQ((*obj.data_diff)["2"].text, "a");
            EXPECT_EQ((*obj.data_diff)["2"].on_click_member, "x");
            EXPECT_EQ((*obj.data_diff)["2"].on_click_field, "y");
        },
        [&] { ADD_FAILURE() << "View recv error"; });
    dummy_s->recvClear();

    data_->view_store.setSend(
        "a", std::make_shared<std::vector<ViewComponentBase>>(
                 std::vector<ViewComponentBase>{
                     ViewComponents::text("b")
                         .textColor(ViewColor::red)
                         .bgColor(ViewColor::green)
                         .lockTmp(data_, ""),
                     ViewComponents::newLine().lockTmp(data_, ""),
                     ViewComponents::button("a", Func{Field{data_, "x", "y"}})
                         .lockTmp(data_, ""),
                 }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::View>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            EXPECT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ((*obj.data_diff)["0"].type, ViewComponentType::text);
            EXPECT_EQ((*obj.data_diff)["0"].text, "b");
            EXPECT_EQ((*obj.data_diff)["0"].text_color, ViewColor::red);
            EXPECT_EQ((*obj.data_diff)["0"].bg_color, ViewColor::green);
        },
        [&] { ADD_FAILURE() << "View recv error"; });
}
TEST_F(ClientTest, viewReq) {
    data_->view_store.getRecv("a", "b");
    wcli_->sync();
    wait();
    wcli_->member("a").view("b").appendListener(callback<View>());
    dummy_s->recv<Message::Req<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "View Req recv error"; });

    auto v = std::make_shared<
        std::unordered_map<std::string, Message::View::ViewComponent>>(
        std::unordered_map<std::string, Message::View::ViewComponent>{
            {"0", ViewComponents::text("a")
                      .textColor(ViewColor::yellow)
                      .bgColor(ViewColor::green)
                      .lockTmp(data_, "")},
            {"1", ViewComponents::newLine().lockTmp(data_, "")},
            {"2", ViewComponents::button("a", Func{Field{data_, "x", "y"}})
                      .lockTmp(data_, "")},
        });
    dummy_s->send(Message::Res<Message::View>{1, "", v, 3});
    dummy_s->send(Message::Res<Message::View>{1, "c", v, 3});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->view_store.getRecv("a", "b").has_value());
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->size(), 3);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).type_,
              ViewComponentType::text);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).text_, "a");
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).text_color_,
              ViewColor::yellow);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).bg_color_,
              ViewColor::green);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(1).type_,
              ViewComponentType::new_line);
    EXPECT_TRUE(data_->view_store.getRecv("a", "b.c").has_value());

    // 差分だけ送る
    auto v2 = std::make_shared<
        std::unordered_map<std::string, Message::View::ViewComponent>>(
        std::unordered_map<std::string, Message::View::ViewComponent>{
            {"0", ViewComponents::text("b")
                      .textColor(ViewColor::red)
                      .bgColor(ViewColor::green)
                      .lockTmp(data_, "")},
        });
    dummy_s->send(Message::Res<Message::View>{1, "", v2, 3});
    wait();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->size(), 3);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).type_,
              ViewComponentType::text);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).text_, "b");
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).text_color_,
              ViewColor::red);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(0).bg_color_,
              ViewColor::green);
    EXPECT_EQ(data_->view_store.getRecv("a", "b").value()->at(1).type_,
              ViewComponentType::new_line);
}
TEST_F(ClientTest, logSend) {
    data_->logger_sink->push(
        std::make_shared<LogLine>(0, std::chrono::system_clock::now(), "a"));
    data_->logger_sink->push(
        std::make_shared<LogLine>(1, std::chrono::system_clock::now(), "b"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.log->size(), 2);
            EXPECT_EQ(obj.log->at(0).level, 0);
            EXPECT_EQ(obj.log->at(0).message, "a");
            EXPECT_EQ(obj.log->at(1).level, 1);
            EXPECT_EQ(obj.log->at(1).message, "b");
        },
        [&] { ADD_FAILURE() << "Log recv error"; });

    dummy_s->recvClear();
    data_->logger_sink->push(
        std::make_shared<LogLine>(2, std::chrono::system_clock::now(), "c"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.log->size(), 1);
            EXPECT_EQ(obj.log->at(0).level, 2);
            EXPECT_EQ(obj.log->at(0).message, "c");
        },
        [&] { ADD_FAILURE() << "Log recv error"; });
}
TEST_F(ClientTest, logReq) {
    data_->log_store.getRecv("a");
    wcli_->sync();
    wait();
    wcli_->member("a").log().appendListener(callback<Log>());
    dummy_s->recv<Message::LogReq>(
        [&](const auto &obj) { EXPECT_EQ(obj.member, "a"); },
        [&] { ADD_FAILURE() << "Log Req recv error"; });

    dummy_s->send(Message::SyncInit{{}, "a", 10, "", "", ""});
    dummy_s->send(
        Message::Log{{},
                     10,
                     std::make_shared<std::vector<Message::Log::LogLine>>(
                         std::vector<Message::Log::LogLine>{
                             LogLine{0, std::chrono::system_clock::now(), "a"},
                             LogLine{1, std::chrono::system_clock::now(), "b"},
                         })});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->log_store.getRecv("a").has_value());
    EXPECT_EQ(data_->log_store.getRecv("a").value()->size(), 2);
    EXPECT_EQ(data_->log_store.getRecv("a").value()->at(0)->level, 0);
    EXPECT_EQ(data_->log_store.getRecv("a").value()->at(0)->message, "a");

    dummy_s->send(
        Message::Log{{},
                     10,
                     std::make_shared<std::vector<Message::Log::LogLine>>(
                         std::vector<Message::Log::LogLine>{
                             LogLine{2, std::chrono::system_clock::now(), "c"},
                         })});
    wait();
    EXPECT_EQ(callback_called, 2);
    EXPECT_TRUE(data_->log_store.getRecv("a").has_value());
    EXPECT_EQ(data_->log_store.getRecv("a").value()->size(), 3);
}
TEST_F(ClientTest, funcInfo) {
    auto f =
        wcli_->func("a").set([](int) { return 1; }).setArgs({Arg("a").init(3)});
    wcli_->sync();
    wait();
    dummy_s->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.return_type, ValType::int_);
            EXPECT_EQ(obj.args->size(), 1);
            EXPECT_EQ(obj.args->at(0).name(), "a");
        },
        [&] { ADD_FAILURE() << "FuncInfo recv error"; });
}
TEST_F(ClientTest, funcCall) {
    // call
    dummy_s->send(Message::SyncInit{{}, "a", 10, "", "", ""});
    wait();
    auto r = wcli_->member("a").func("b").runAsync(1, true, "a");
    wait();
    dummy_s->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 0);
            EXPECT_EQ(obj.target_member_id, 10);
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.args.size(), 3);
            EXPECT_EQ(static_cast<int>(obj.args[0]), 1);
            EXPECT_EQ(obj.args[0].valType(), ValType::int_);
            EXPECT_EQ(static_cast<bool>(obj.args[1]), true);
            EXPECT_EQ(obj.args[1].valType(), ValType::bool_);
            EXPECT_EQ(static_cast<std::string>(obj.args[2]), "a");
            EXPECT_EQ(obj.args[2].valType(), ValType::string_);
        },
        [&] { ADD_FAILURE() << "FuncInfo recv error"; });

    // started=false
    dummy_s->send(Message::CallResponse{{}, 0, 0, false});
    wait();
    EXPECT_FALSE(r.started.get());
    EXPECT_THROW(r.result.get(), FuncNotFound);
    dummy_s->recvClear();

    // 2nd call id=1
    r = wcli_->member("a").func("b").runAsync(1, true, "a");
    wait();
    dummy_s->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 1);
            EXPECT_EQ(obj.target_member_id, 10);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "FuncInfo recv error"; });

    // started=true
    dummy_s->send(Message::CallResponse{{}, 1, 0, true});
    wait();
    EXPECT_TRUE(r.started.get());
    // return error
    dummy_s->send(Message::CallResult{{}, 1, 0, true, "a"});
    wait();
    EXPECT_THROW(r.result.get(), std::runtime_error);
    try {
        r.result.get();
    } catch (const std::runtime_error &e) {
        EXPECT_EQ(std::string(e.what()), "a");
    }
    dummy_s->recvClear();

    // 3rd call id=2
    r = wcli_->member("a").func("b").runAsync(1, true, "a");
    wait();
    dummy_s->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 2);
            EXPECT_EQ(obj.target_member_id, 10);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "FuncInfo recv error"; });

    // started=true
    dummy_s->send(Message::CallResponse{{}, 2, 0, true});
    wait();
    // return
    dummy_s->send(Message::CallResult{{}, 2, 0, false, "b"});
    wait();
    EXPECT_EQ(static_cast<std::string>(r.result.get()), "b");
}
TEST_F(ClientTest, funcResponse) {
    wcli_->func("a").set([](int a) {
        if (a == 0) {
            throw std::invalid_argument("a==0");
        } else {
            return a;
        }
    });
    // not found
    dummy_s->send(Message::Call{FuncCall{7, 100, 0, "n", {}}});
    wait();
    dummy_s->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 7);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.started, false);
        },
        [&] { ADD_FAILURE() << "CallResponse recv error"; });
    dummy_s->recvClear();

    // arg error
    dummy_s->send(Message::Call{FuncCall{8, 100, 0, "a", {1, "zzz"}}});
    wait();
    dummy_s->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 8);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.started, true);
        },
        [&] { ADD_FAILURE() << "CallResponse recv error"; });
    dummy_s->recv<Message::CallResult>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 8);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.is_error, true);
            EXPECT_EQ(static_cast<std::string>(obj.result),
                      "requires 1 arguments, got 2");
        },
        [&] { ADD_FAILURE() << "CallResult recv error"; });
    dummy_s->recvClear();

    // throw
    dummy_s->send(Message::Call{FuncCall{9, 100, 0, "a", {0}}});
    wait();
    dummy_s->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 9);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.started, true);
        },
        [&] { ADD_FAILURE() << "CallResponse recv error"; });
    dummy_s->recv<Message::CallResult>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 9);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.is_error, true);
            // 関数の中でthrowされた内容
            EXPECT_EQ(static_cast<std::string>(obj.result), "a==0");
        },
        [&] { ADD_FAILURE() << "CallResult recv error"; });
    dummy_s->recvClear();

    // success
    dummy_s->send(Message::Call{FuncCall{19, 100, 0, "a", {123}}});
    wait();
    dummy_s->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 19);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.started, true);
        },
        [&] { ADD_FAILURE() << "CallResponse recv error"; });
    dummy_s->recv<Message::CallResult>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 19);
            EXPECT_EQ(obj.caller_member_id, 100);
            EXPECT_EQ(obj.is_error, false);
            // 関数の中でthrowされた内容
            EXPECT_EQ(static_cast<int>(obj.result), 123);
        },
        [&] { ADD_FAILURE() << "CallResult recv error"; });
    dummy_s->recvClear();
}