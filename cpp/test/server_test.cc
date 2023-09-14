#include <gtest/gtest.h>
#include "../server/websock.h"
#include "../server/store.h"
#include "../server/s_client_data.h"
#include "../message/message.h"
#include <spdlog/sinks/stdout_color_sinks.h>
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
    EXPECT_EQ(Server::store.clients_by_id.at(1)->name, "");
    EXPECT_EQ(Server::store.clients_by_id.at(2)->name, "c2");
    dummy_c1->recvClear();

    // dummy_c2->send(Message::Sync{});
    // wait();
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
    dummy_c1->send(
        Message::FuncInfo{"a", FuncInfo{std::function<void()>(), nullptr}});
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
    dummy_c1->send(
        Message::FuncInfo{"b", FuncInfo{std::function<void()>(), nullptr}});
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
