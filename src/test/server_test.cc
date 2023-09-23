#include <gtest/gtest.h>
#include "../server/websock.h"
#include "../server/store.h"
#include "../server/s_client_data.h"
#include "../message/message.h"
#include <webcface/common/def.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <webcface/view.h>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace WebCFace;

static void wait(int ms = 10) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class ServerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        Server::store.clear();
        auto stderr_sink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        server_thread = std::make_shared<std::thread>(
            Server::serverRun, 27530, stderr_sink, spdlog::level::trace);
        wait();
        dummy_c1 = std::make_shared<DummyClient>();
        wait();
        dummy_c2 = std::make_shared<DummyClient>();
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown" << std::endl;
        Server::serverStop();
        server_thread->join();
    }
    std::shared_ptr<std::thread> server_thread;
    std::shared_ptr<DummyClient> dummy_c1, dummy_c2;
    int callback_called;
};

TEST_F(ServerTest, connection) { EXPECT_EQ(Server::store.clients.size(), 2); }
TEST_F(ServerTest, sync) {
    dummy_c1->send(Message::SyncInit{{}, "", 0});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "c2", 0});
    wait();
    dummy_c1->recv<Message::SyncInit>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_name, "c2");
            EXPECT_EQ(obj.member_id, 2);
        },
        [&] { ADD_FAILURE() << "SyncInit recv failed"; });
    dummy_c1->recv<Message::SvrVersion>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.svr_name, WEBCFACE_SERVER_NAME);
            EXPECT_EQ(obj.ver, WEBCFACE_VERSION);
        },
        [&] { ADD_FAILURE() << "SvrVersion recv failed"; });
    EXPECT_EQ(Server::store.clients_by_id.at(1)->name, "");
    EXPECT_EQ(Server::store.clients_by_id.at(2)->name, "c2");
    dummy_c1->recvClear();

    // dummy_c2->send(Message::Sync{});
    // wait();
}
TEST_F(ServerTest, ping) {
    dummy_c1->send(Message::SyncInit{{}, "", 0});
    wait();
    Server::server_stop_cond.notify_one(); // これで無理やりpingさせる
    auto s_c1 = Server::store.clients_by_id.at(1);
    wait(10);
    dummy_c1->recv<Message::Ping>([&] {},
                                  [&] { ADD_FAILURE() << "Ping recv failed"; });
    dummy_c1->send(Message::Ping{});
    wait();
    EXPECT_TRUE(s_c1->last_ping_duration.has_value());
    EXPECT_GE(s_c1->last_ping_duration->count(), 10);
    EXPECT_LE(s_c1->last_ping_duration->count(), 12);

    dummy_c1->send(Message::PingStatusReq{});
    dummy_c1->recvClear();
    wait();
    Server::server_stop_cond.notify_one(); // これで無理やりpingさせる
    wait();
    dummy_c1->recv<Message::PingStatus>(
        [&](auto &obj) {
            EXPECT_TRUE(obj.status->count(1));
            EXPECT_GE(obj.status->at(1), 10);
            EXPECT_LE(obj.status->at(1), 12);
        },
        [&] { ADD_FAILURE() << "Ping Status recv failed"; });
}
TEST_F(ServerTest, entry) {
    dummy_c1->send(Message::SyncInit{{}, "c1", 0});
    dummy_c1->send(
        Message::Value{{}, "a", std::make_shared<std::vector<double>>(1)});
    dummy_c1->send(Message::Text{{}, "a", std::make_shared<std::string>("")});
    dummy_c1->send(Message::View{
        "a",
        std::make_shared<
            std::unordered_map<int, Message::View::ViewComponent>>(),
        0});
    dummy_c1->send(Message::FuncInfo{
        0, "a", ValType::none_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>()});
    wait();
    // c2が接続したタイミングでのc1のentryが全部返る
    dummy_c2->send(Message::SyncInit{{}, "", 0});
    wait();
    dummy_c2->recv<Message::SyncInit>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_name, "c1");
            EXPECT_EQ(obj.member_id, 1);
        },
        [&] { ADD_FAILURE() << "SyncInit recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "Value Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "Text Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "View Entry recv failed"; });
    dummy_c2->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.return_type, ValType::none_);
            EXPECT_EQ(obj.args->size(), 0);
        },
        [&] { ADD_FAILURE() << "Func Info recv failed"; });
    dummy_c2->recvClear();

    // c1にentryを追加する
    dummy_c1->send(
        Message::Value{{}, "b", std::make_shared<std::vector<double>>(1)});
    wait();
    dummy_c2->recv<Message::Entry<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "Value Entry recv failed"; });
    dummy_c1->send(Message::Text{{}, "b", std::make_shared<std::string>("")});
    wait();
    dummy_c2->recv<Message::Entry<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "Text Entry recv failed"; });
    dummy_c1->send(Message::View{
        "b",
        std::make_shared<
            std::unordered_map<int, Message::View::ViewComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "View Entry recv failed"; });
    dummy_c1->send(Message::FuncInfo{
        0, "b", ValType::none_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>()});
    wait();
    dummy_c2->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.return_type, ValType::none_);
            EXPECT_EQ(obj.args->size(), 0);
        },
        [&] { ADD_FAILURE() << "Func Info recv failed"; });
}
TEST_F(ServerTest, value) {
    dummy_c1->send(Message::SyncInit{{}, "c1", 0});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Value{
        {},
        "a",
        std::make_shared<std::vector<double>>(std::vector<double>{3, 4, 5})});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0});
    dummy_c2->send(Message::Req<Message::Value>{{}, "c1", "a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data->size(), 3);
            EXPECT_EQ(obj.data->at(0), 3);
        },
        [&] { ADD_FAILURE() << "Value Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Value{{},
                                  "a",
                                  std::make_shared<std::vector<double>>(
                                      std::vector<double>{6, 7, 8, 9})});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data->size(), 4);
            EXPECT_EQ(obj.data->at(0), 6);
        },
        [&] { ADD_FAILURE() << "Value Res recv failed"; });
}
TEST_F(ServerTest, text) {
    dummy_c1->send(Message::SyncInit{{}, "c1", 0});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(
        Message::Text{{}, "a", std::make_shared<std::string>("zzz")});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0});
    dummy_c2->send(Message::Req<Message::Text>{{}, "c1", "a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(*obj.data, "zzz");
        },
        [&] { ADD_FAILURE() << "Text Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(
        Message::Text{{}, "a", std::make_shared<std::string>("zzzzz")});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(*obj.data, "zzzzz");
        },
        [&] { ADD_FAILURE() << "Text Res recv failed"; });
}
TEST_F(ServerTest, view) {
    dummy_c1->send(Message::SyncInit{{}, "c1", 0});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::View{
        "a",
        std::make_shared<std::unordered_map<int, Message::View::ViewComponent>>(
            std::unordered_map<int, Message::View::ViewComponent>{
                {0, ViewComponents::text("a")},
                {1, ViewComponents::newLine()},
                {2, ViewComponents::button(
                        "f",
                        Func{Field{std::weak_ptr<ClientData>(), "p", "q"}})}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0});
    dummy_c2->send(Message::Req<Message::View>{{}, "c1", "a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.data_diff->at(0).type, ViewComponentType::text);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "View Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::View{
        "a",
        std::make_shared<std::unordered_map<int, Message::View::ViewComponent>>(
            std::unordered_map<int, Message::View::ViewComponent>{
                {0, ViewComponents::text("b")},
            }),
        3});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ(obj.data_diff->at(0).type, ViewComponentType::text);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "View Res recv failed"; });
}
TEST_F(ServerTest, log) {
    dummy_c1->send(Message::SyncInit{{}, "c1", 0});
    dummy_c1->send(
        Message::Log{{},
                     0,
                     std::make_shared<std::vector<Message::Log::LogLine>>(
                         std::vector<Message::Log::LogLine>{
                             LogLine{0, std::chrono::system_clock::now(), "0"},
                             LogLine{1, std::chrono::system_clock::now(), "1"},
                         })});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0});
    dummy_c2->send(Message::LogReq{{}, "c1"});
    wait();
    // req時の値
    dummy_c2->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.log->size(), 2);
            EXPECT_EQ(obj.log->at(0).level, 0);
            EXPECT_EQ(obj.log->at(0).message, "0");
        },
        [&] { ADD_FAILURE() << "Log recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(
        Message::Log{{},
                     0,
                     std::make_shared<std::vector<Message::Log::LogLine>>(
                         std::vector<Message::Log::LogLine>{
                             LogLine{2, std::chrono::system_clock::now(), "2"},
                         })});
    wait();
    dummy_c2->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.log->size(), 1);
            EXPECT_EQ(obj.log->at(0).level, 2);
            EXPECT_EQ(obj.log->at(0).message, "2");
        },
        [&] { ADD_FAILURE() << "Log recv failed"; });
}
TEST_F(ServerTest, call) {
    dummy_c1->send(Message::SyncInit{{}, "c1", 0});
    dummy_c2->send(Message::SyncInit{{}, "c2", 0});
    wait();
    // c2がc1にcallを送る (caller_id=1)
    dummy_c2->send(Message::Call{FuncCall{1, 0, 1, "a", {0, 0, 0}}});
    wait();
    dummy_c1->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 1);
            EXPECT_EQ(obj.caller_member_id, 2);
            EXPECT_EQ(obj.target_member_id, 1);
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.args.size(), 3);
        },
        [&] { ADD_FAILURE() << "Call recv failed"; });
    dummy_c2->recvClear();

    dummy_c1->send(Message::CallResponse{{}, 1, 2, true});
    wait();
    dummy_c2->recv<Message::CallResponse>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 1);
            EXPECT_EQ(obj.caller_member_id, 2);
            EXPECT_EQ(obj.started, true);
        },
        [&] { ADD_FAILURE() << "Call Response recv failed"; });
    dummy_c2->recvClear();

    dummy_c1->send(Message::CallResult{{}, 1, 2, false, "aaa"});
    wait();
    dummy_c2->recv<Message::CallResult>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 1);
            EXPECT_EQ(obj.caller_member_id, 2);
            EXPECT_EQ(obj.is_error, false);
            EXPECT_EQ(static_cast<std::string>(obj.result), "aaa");
        },
        [&] { ADD_FAILURE() << "Call Result recv failed"; });
}