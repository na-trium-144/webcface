#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/server.h>
#include "../server/store.h"
#include "../server/member_data.h"
#include "../message/message.h"
#include <webcface/common/def.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <webcface/view.h>
#include <webcface/image.h>
#include <webcface/robot_model.h>
#include <webcface/canvas3d.h>
#include <webcface/canvas2d.h>
#include <thread>
#include <iostream>
#include "dummy_client.h"
#include "webcface/common/image.h"

using namespace webcface;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}

class ServerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        auto stderr_sink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        server = std::make_unique<Server::Server>(27530, stderr_sink,
                                                  spdlog::level::trace);
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown begin" << std::endl;
        dummy_c1.reset();
        dummy_c2.reset();
        server.reset();
        std::cout << "TearDown end" << std::endl;
    }
    std::unique_ptr<Server::Server> server;
    std::shared_ptr<Internal::ClientData> data_ =
        std::make_shared<Internal::ClientData>(u8"a");
    std::shared_ptr<DummyClient> dummy_c1, dummy_c2;
    int callback_called;
};

TEST_F(ServerTest, connection) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    EXPECT_EQ(server->store->clients.size(), 2);
}
TEST_F(ServerTest, unixSocketConnection) {
    dummy_c1 = std::make_shared<DummyClient>(true);
    wait();
    dummy_c2 = std::make_shared<DummyClient>(true);
    wait();
    EXPECT_EQ(server->store->clients.size(), 2);
}
TEST_F(ServerTest, sync) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"c2", 0, "a", "1", ""});
    wait();
    dummy_c1->recv<Message::SyncInit>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_name, u8"c2");
            EXPECT_EQ(obj.member_id, 2);
            EXPECT_EQ(obj.lib_name, "a");
            EXPECT_EQ(obj.lib_ver, "1");
            EXPECT_EQ(obj.addr, "127.0.0.1");
        },
        [&] { ADD_FAILURE() << "SyncInit recv failed"; });
    dummy_c1->recv<Message::SvrVersion>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.svr_name, WEBCFACE_SERVER_NAME);
            EXPECT_EQ(obj.ver, WEBCFACE_VERSION);
        },
        [&] { ADD_FAILURE() << "SvrVersion recv failed"; });
    ASSERT_TRUE(server->store->clients_by_id.count(1));
    ASSERT_TRUE(server->store->clients_by_id.count(2));
    EXPECT_EQ(server->store->clients_by_id.at(1)->name, u8"");
    EXPECT_EQ(server->store->clients_by_id.at(2)->name, u8"c2");
    dummy_c1->recvClear();

    // dummy_c2->send(Message::Sync{});
    // wait();
}
TEST_F(ServerTest, ping) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    wait();
    auto start = std::chrono::steady_clock::now();
    server->server_ping_wait.notify_one(); // これで無理やりpingさせる
    auto s_c1 = server->store->clients_by_id.at(1);
    wait();
    dummy_c1->recv<Message::Ping>([&](const auto &) {},
                                  [&] { ADD_FAILURE() << "Ping recv failed"; });
    dummy_c1->send(Message::Ping{});
    wait();
    auto end = std::chrono::steady_clock::now();
    auto dur_max =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    EXPECT_TRUE(s_c1->last_ping_duration.has_value());
    EXPECT_GE(s_c1->last_ping_duration->count(), WEBCFACE_TEST_TIMEOUT - 1);
    EXPECT_LE(s_c1->last_ping_duration->count(), dur_max);

    // serverがping statusを集計するのは次のping時なのでこの場合0
    dummy_c1->recvClear();
    dummy_c1->send(Message::PingStatusReq{});
    wait();
    dummy_c1->recv<Message::PingStatus>(
        [&](const auto &obj) { EXPECT_EQ(obj.status->size(), 0); },
        [&] { ADD_FAILURE() << "Ping Status recv failed"; });

    dummy_c1->recvClear();
    server->server_ping_wait.notify_one(); // これで無理やりpingさせる
    wait();
    dummy_c1->recv<Message::PingStatus>(
        [&](const auto &obj) {
            EXPECT_TRUE(obj.status->count(1));
            EXPECT_GE(obj.status->at(1), WEBCFACE_TEST_TIMEOUT - 1);
            EXPECT_LE(obj.status->at(1), dur_max);
        },
        [&] { ADD_FAILURE() << "Ping Status recv failed"; });
}
TEST_F(ServerTest, entry) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(
        Message::Value{{}, u8"a", std::make_shared<std::vector<double>>(1)});
    dummy_c1->send(Message::Text{{}, u8"a", std::make_shared<ValAdaptor>("")});
    dummy_c1->send(Message::RobotModel{
        u8"a", std::make_shared<std::vector<Common::RobotLink>>()});
    dummy_c1->send(Message::Canvas3D{
        u8"a",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(),
        0});
    dummy_c1->send(Message::Canvas2D{
        u8"a", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(),
        0});
    dummy_c1->send(Message::View{
        u8"a",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(),
        0});
    dummy_c1->send(Message::Image{
        u8"a", ImageFrame{100, 100,
                          std::make_shared<std::vector<unsigned char>>(
                              100 * 100 * 3)}});
    dummy_c1->send(Message::FuncInfo{
        0, u8"a", ValType::none_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>()});
    wait();
    // c2が接続したタイミングでのc1のentryが全部返る
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    wait();
    dummy_c2->recv<Message::SyncInit>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_name, u8"c1");
            EXPECT_EQ(obj.member_id, 1);
        },
        [&] { ADD_FAILURE() << "SyncInit recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "Value Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "Text Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "RobotModel Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "View Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "Canvas3D Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "Canvas2D Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
        },
        [&] { ADD_FAILURE() << "Image Entry recv failed"; });
    dummy_c2->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
            EXPECT_EQ(obj.return_type, ValType::none_);
            EXPECT_EQ(obj.args->size(), 0);
        },
        [&] { ADD_FAILURE() << "Func Info recv failed"; });
    dummy_c2->recvClear();

    // c1にentryを追加する
    dummy_c1->send(
        Message::Value{{}, u8"b", std::make_shared<std::vector<double>>(1)});
    wait();
    dummy_c2->recv<Message::Entry<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "Value Entry recv failed"; });
    dummy_c1->send(Message::Text{{}, u8"b", std::make_shared<ValAdaptor>("")});
    wait();
    dummy_c2->recv<Message::Entry<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "Text Entry recv failed"; });
    dummy_c1->send(Message::RobotModel{
        u8"b", std::make_shared<std::vector<Common::RobotLink>>()});
    wait();
    dummy_c2->recv<Message::Entry<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "RobotModel Entry recv failed"; });
    dummy_c1->send(Message::View{
        u8"b",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "View Entry recv failed"; });
    dummy_c1->send(Message::Canvas3D{
        u8"b",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "Canvas3D Entry recv failed"; });
    dummy_c1->send(Message::Canvas2D{
        u8"b", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "Canvas2D Entry recv failed"; });
    dummy_c1->send(Message::Image{
        u8"b",
        ImageFrame{50, 50,
                   std::make_shared<std::vector<unsigned char>>(50 * 50 * 3)}});
    wait();
    dummy_c2->recv<Message::Entry<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
        },
        [&] { ADD_FAILURE() << "Image Entry recv failed"; });

    dummy_c1->send(Message::FuncInfo{
        0, u8"b", ValType::none_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>()});
    wait();
    dummy_c2->recv<Message::FuncInfo>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, u8"b");
            EXPECT_EQ(obj.return_type, ValType::none_);
            EXPECT_EQ(obj.args->size(), 0);
        },
        [&] { ADD_FAILURE() << "Func Info recv failed"; });
}
TEST_F(ServerTest, value) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Value{
        {},
        u8"a",
        std::make_shared<std::vector<double>>(std::vector<double>{3, 4, 5})});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::Value>{{}, u8"c1", u8"a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data->size(), 3);
            EXPECT_EQ(obj.data->at(0), 3);
        },
        [&] { ADD_FAILURE() << "Value Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Value{{},
                                  u8"a",
                                  std::make_shared<std::vector<double>>(
                                      std::vector<double>{6, 7, 8, 9})});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data->size(), 4);
            EXPECT_EQ(obj.data->at(0), 6);
        },
        [&] { ADD_FAILURE() << "Value Res recv failed"; });
}
TEST_F(ServerTest, text) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(
        Message::Text{{}, u8"a", std::make_shared<ValAdaptor>("zzz")});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::Text>{{}, u8"c1", u8"a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(*obj.data, "zzz");
        },
        [&] { ADD_FAILURE() << "Text Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(
        Message::Text{{}, u8"a", std::make_shared<ValAdaptor>("zzzzz")});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(*obj.data, "zzzzz");
        },
        [&] { ADD_FAILURE() << "Text Res recv failed"; });
}
TEST_F(ServerTest, robotModel) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::RobotModel{
        u8"a", std::make_shared<std::vector<RobotLink>>(std::vector<RobotLink>{
                   {"a", Geometry{}, ViewColor::black}})});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::RobotModel>{{}, u8"c1", u8"a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.commonLinks()->size(), 1);
        },
        [&] { ADD_FAILURE() << "RobotModel Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::RobotModel{
        u8"a", std::make_shared<std::vector<RobotLink>>(std::vector<RobotLink>{
                   RobotLink{"a", {}, Geometry{}, ViewColor::black},
                   RobotLink{"b", {}, Geometry{}, ViewColor::black},
                   RobotLink{"c",
                             {u8"j", u8"a", RobotJointType::fixed, {}, 0},
                             Geometry{},
                             ViewColor::black},
               })});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            ASSERT_EQ(obj.commonLinks()->size(), 3);
            EXPECT_EQ(obj.commonLinks()->at(0).joint.parent_name, u8"");
            EXPECT_EQ(obj.commonLinks()->at(1).joint.parent_name, u8"");
            EXPECT_EQ(obj.commonLinks()->at(2).joint.parent_name, u8"a");
        },
        [&] { ADD_FAILURE() << "RobotModel Res recv failed"; });
}
TEST_F(ServerTest, view) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::View{
        u8"a",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(
            std::unordered_map<std::string, Message::View::ViewComponent>{
                {"0", ViewComponents::text("a").toV().lockTmp(data_, u8"")},
                {"1", ViewComponents::newLine().lockTmp(data_, u8"")},
                {"2", ViewComponents::button(
                          "f", Func{Field{std::weak_ptr<Internal::ClientData>(),
                                          u8"p", u8"q"}})
                          .lockTmp(data_, u8"")}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::View>{{}, u8"c1", u8"a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.data_diff->at("0").type, ViewComponentType::text);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "View Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::View{
        u8"a",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(
            std::unordered_map<std::string, Message::View::ViewComponent>{
                {"0", ViewComponents::text("b").toV().lockTmp(data_, u8"")},
            }),
        3});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ(obj.data_diff->at("0").type, ViewComponentType::text);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "View Res recv failed"; });
}
TEST_F(ServerTest, canvas3d) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas3D{
        u8"a",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(
            std::unordered_map<std::string,
                               Message::Canvas3D::Canvas3DComponent>{
                {"0", {}}, {"1", {}}, {"2", {}}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::Canvas3D>{{}, u8"c1", u8"a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "Canvas3D Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas3D{
        u8"a",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(
            std::unordered_map<std::string,
                               Message::Canvas3D::Canvas3DComponent>{
                {"0", {}},
            }),
        3});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "Canvas3D Res recv failed"; });
}
TEST_F(ServerTest, canvas2d) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas2D{
        u8"a", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(
            std::unordered_map<std::string,
                               Message::Canvas2D::Canvas2DComponent>{
                {"0", {}}, {"1", {}}, {"2", {}}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::Canvas2D>{{}, u8"c1", u8"a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "Canvas2D Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas2D{
        u8"a", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(
            std::unordered_map<std::string,
                               Message::Canvas2D::Canvas2DComponent>{
                {"0", {}},
            }),
        3});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "Canvas2D Res recv failed"; });
}
TEST_F(ServerTest, image) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    auto sendImage = [&] {
        dummy_c1->send(Message::Sync{});
        dummy_c1->send(Message::Image{
            u8"a", ImageFrame{10, 10,
                              std::make_shared<std::vector<unsigned char>>(
                                  10 * 10 * 3)}});
    };
    sendImage();
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});

    // normal request
    dummy_c2->send(Message::Req<Message::Image>{u8"c1", u8"a", 1, {}});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data().size(), 10 * 10 * 3);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    sendImage();
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data().size(), 10 * 10 * 3);
            EXPECT_EQ(obj.rows(), 10);
            EXPECT_EQ(obj.cols(), 10);
            EXPECT_EQ(obj.color_mode(), ImageColorMode::bgr);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 1"; });
    dummy_c2->recvClear();

    // resize, convert color, frame rate
    dummy_c2->send(Message::Req<Message::Image>{
        u8"c1",
        u8"a",
        1,
        {
            5, 5, ImageColorMode::gray, ImageCompressMode::raw, 0,
            1000.0 / WEBCFACE_TEST_TIMEOUT /
                3 // wait()のtimeoutに間に合わないようにする
        }});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data().size(), 5 * 5 * 1);
            EXPECT_EQ(obj.rows(), 5);
            EXPECT_EQ(obj.cols(), 5);
            EXPECT_EQ(obj.color_mode(), ImageColorMode::gray);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 2"; });
    dummy_c2->recvClear();

    sendImage();
    wait();
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](auto) { ADD_FAILURE() << "should not receive Image Res 3"; },
        [] {});
    wait();
    wait();
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.data().size(), 5 * 5 * 1);
            EXPECT_EQ(obj.rows(), 5);
            EXPECT_EQ(obj.cols(), 5);
            EXPECT_EQ(obj.color_mode(), ImageColorMode::gray);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 3"; });
    dummy_c2->recvClear();

    // compress
    dummy_c2->send(Message::Req<Message::Image>{
        u8"c1",
        u8"a",
        1,
        {std::nullopt, std::nullopt, std::nullopt, ImageCompressMode::png, 5,
         std::nullopt}});
    wait();
    wait();
    wait();
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, u8"");
            EXPECT_EQ(obj.rows(), 10);
            EXPECT_EQ(obj.cols(), 10);
            EXPECT_GT(obj.data().size(), 0);
            EXPECT_EQ(obj.compress_mode(), ImageCompressMode::png);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 4"; });
    dummy_c2->recvClear();

    // convert color pattern
    for (int f_type = 0; f_type < 5; f_type++) {
        for (int t_type = 0; t_type < 5; t_type++) {
            dummy_c1->send(Message::Sync{});
            dummy_c1->send(Message::Image{
                Encoding::castToU8("a" + std::to_string(f_type) +
                                   std::to_string(t_type)),
                ImageFrame{10, 10, static_cast<ImageColorMode>(f_type)}});
            wait();
            dummy_c2->send(Message::Req<Message::Image>{
                u8"c1",
                Encoding::castToU8("a" + std::to_string(f_type) +
                                   std::to_string(t_type)),
                1,
                {std::nullopt, std::nullopt,
                 static_cast<ImageColorMode>(t_type), ImageCompressMode::raw, 0,
                 std::nullopt}});
            wait();
            dummy_c2->recv<Message::Res<Message::Image>>(
                [&](const auto &obj) {
                    EXPECT_EQ(obj.req_id, 1);
                    EXPECT_EQ(obj.sub_field, u8"");
                    EXPECT_EQ(obj.rows(), 10);
                    EXPECT_EQ(obj.cols(), 10);
                    EXPECT_EQ(obj.color_mode(),
                              static_cast<ImageColorMode>(t_type));
                },
                [&] {
                    ADD_FAILURE() << "Image Res recv failed from=" << f_type
                                  << ", to=" << t_type;
                });
            dummy_c2->recvClear();
        }
    }
}
TEST_F(ServerTest, log) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    server->store->keep_log = 3;
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c1->send(Message::Log{
        0, std::make_shared<std::deque<Message::Log::LogLine>>(
               std::deque<Message::Log::LogLine>{
                   LogLineData<>{0, std::chrono::system_clock::now(), u8"0"},
                   LogLineData<>{1, std::chrono::system_clock::now(), u8"1"},
                   LogLineData<>{2, std::chrono::system_clock::now(), u8"2"},
                   LogLineData<>{3, std::chrono::system_clock::now(), u8"3"},
               })});
    wait();
    dummy_c2->send(Message::SyncInit{{}, u8"", 0, "", "", ""});
    dummy_c2->send(Message::LogReq{{}, u8"c1"});
    wait();
    // req時の値
    // keep_logを超えたので最後の3行だけ送られる
    dummy_c2->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.log->size(), 3);
            EXPECT_EQ(obj.log->at(0).level, 1);
            EXPECT_EQ(obj.log->at(0).message, u8"1");
        },
        [&] { ADD_FAILURE() << "Log recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Log{
        0, std::make_shared<std::deque<Message::Log::LogLine>>(
               std::deque<Message::Log::LogLine>{
                   LogLineData<>{4, std::chrono::system_clock::now(), u8"4"},
                   LogLineData<>{5, std::chrono::system_clock::now(), u8"5"},
                   LogLineData<>{6, std::chrono::system_clock::now(), u8"6"},
                   LogLineData<>{7, std::chrono::system_clock::now(), u8"7"},
                   LogLineData<>{8, std::chrono::system_clock::now(), u8"8"},
               })});
    wait();
    dummy_c2->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.log->size(), 5);
            EXPECT_EQ(obj.log->at(0).level, 4);
            EXPECT_EQ(obj.log->at(0).message, u8"4");
        },
        [&] { ADD_FAILURE() << "Log recv failed"; });
}
TEST_F(ServerTest, call) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, u8"c1", 0, "", "", ""});
    dummy_c2->send(Message::SyncInit{{}, u8"c2", 0, "", "", ""});
    wait();
    // c2がc1にcallを送る (caller_id=1)
    dummy_c2->send(Message::Call{FuncCall{
        1, 0, 1, u8"a", {ValAdaptor(0), ValAdaptor(0), ValAdaptor(0)}}});
    wait();
    dummy_c1->recv<Message::Call>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.caller_id, 1);
            EXPECT_EQ(obj.caller_member_id, 2);
            EXPECT_EQ(obj.target_member_id, 1);
            EXPECT_EQ(obj.field, u8"a");
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

    dummy_c1->send(Message::CallResult{{}, 1, 2, false, ValAdaptor("aaa")});
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
