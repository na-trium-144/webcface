#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include "../server/websock.h"
#include "../server/store.h"
#include "../server/s_client_data.h"
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
        Server::store.clear();
        auto stderr_sink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        server_thread = std::make_shared<std::thread>(
            Server::serverRun, 27530, stderr_sink, spdlog::level::trace);
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown begin" << std::endl;
        dummy_c1.reset();
        dummy_c2.reset();
        Server::serverStop();
        server_thread->join();
        server_thread.reset();
        std::cout << "TearDown end" << std::endl;
    }
    std::shared_ptr<std::thread> server_thread;
    std::shared_ptr<Internal::ClientData> data_ =
        std::make_shared<Internal::ClientData>("a");
    std::shared_ptr<DummyClient> dummy_c1, dummy_c2;
    int callback_called;
};

TEST_F(ServerTest, connection) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    EXPECT_EQ(Server::store.clients.size(), 2);
}
TEST_F(ServerTest, unixSocketConnection) {
    dummy_c1 = std::make_shared<DummyClient>(true);
    wait();
    dummy_c2 = std::make_shared<DummyClient>(true);
    wait();
    EXPECT_EQ(Server::store.clients.size(), 2);
}
TEST_F(ServerTest, sync) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "", 0, "", "", ""});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "c2", 0, "a", "1", ""});
    wait();
    dummy_c1->recv<Message::SyncInit>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_name, "c2");
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
    EXPECT_EQ(Server::store.clients_by_id.at(1)->name, "");
    EXPECT_EQ(Server::store.clients_by_id.at(2)->name, "c2");
    dummy_c1->recvClear();

    // dummy_c2->send(Message::Sync{});
    // wait();
}
TEST_F(ServerTest, ping) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "", 0, "", "", ""});
    wait();
    auto start = std::chrono::steady_clock::now();
    Server::server_ping_wait.notify_one(); // これで無理やりpingさせる
    auto s_c1 = Server::store.clients_by_id.at(1);
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
    EXPECT_GE(s_c1->last_ping_duration->count(), 10);
    EXPECT_LE(s_c1->last_ping_duration->count(), dur_max);

    // serverがping statusを集計するのは次のping時なのでこの場合0
    dummy_c1->recvClear();
    dummy_c1->send(Message::PingStatusReq{});
    wait();
    dummy_c1->recv<Message::PingStatus>(
        [&](const auto &obj) { EXPECT_EQ(obj.status->size(), 0); },
        [&] { ADD_FAILURE() << "Ping Status recv failed"; });

    dummy_c1->recvClear();
    Server::server_ping_wait.notify_one(); // これで無理やりpingさせる
    wait();
    dummy_c1->recv<Message::PingStatus>(
        [&](const auto &obj) {
            EXPECT_TRUE(obj.status->count(1));
            EXPECT_GE(obj.status->at(1), 10);
            EXPECT_LE(obj.status->at(1), dur_max);
        },
        [&] { ADD_FAILURE() << "Ping Status recv failed"; });
}
TEST_F(ServerTest, entry) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(
        Message::Value{{}, "a", std::make_shared<std::vector<double>>(1)});
    dummy_c1->send(Message::Text{{}, "a", std::make_shared<ValAdaptor>("")});
    dummy_c1->send(Message::RobotModel{
        "a", std::make_shared<std::vector<Common::RobotLink>>()});
    dummy_c1->send(Message::Canvas3D{
        "a",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(),
        0});
    dummy_c1->send(Message::Canvas2D{
        "a", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(),
        0});
    dummy_c1->send(Message::View{
        "a",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(),
        0});
    dummy_c1->send(Message::Image{
        "a", ImageFrame{
                 100, 100,
                 std::make_shared<std::vector<unsigned char>>(100 * 100 * 3)}});
    dummy_c1->send(Message::FuncInfo{
        0, "a", ValType::none_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>()});
    wait();
    // c2が接続したタイミングでのc1のentryが全部返る
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
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
    dummy_c2->recv<Message::Entry<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "RobotModel Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "View Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "Canvas3D Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "Canvas2D Entry recv failed"; });
    dummy_c2->recv<Message::Entry<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a");
        },
        [&] { ADD_FAILURE() << "Image Entry recv failed"; });
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
    dummy_c1->send(Message::Text{{}, "b", std::make_shared<ValAdaptor>("")});
    wait();
    dummy_c2->recv<Message::Entry<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "Text Entry recv failed"; });
    dummy_c1->send(Message::RobotModel{
        "b", std::make_shared<std::vector<Common::RobotLink>>()});
    wait();
    dummy_c2->recv<Message::Entry<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "RobotModel Entry recv failed"; });
    dummy_c1->send(Message::View{
        "b",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "View Entry recv failed"; });
    dummy_c1->send(Message::Canvas3D{
        "b",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "Canvas3D Entry recv failed"; });
    dummy_c1->send(Message::Canvas2D{
        "b", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(),
        0});
    wait();
    dummy_c2->recv<Message::Entry<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "Canvas2D Entry recv failed"; });
    dummy_c1->send(Message::Image{
        "b",
        ImageFrame{50, 50,
                   std::make_shared<std::vector<unsigned char>>(50 * 50 * 3)}});
    wait();
    dummy_c2->recv<Message::Entry<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b");
        },
        [&] { ADD_FAILURE() << "Image Entry recv failed"; });

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
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Value{
        {},
        "a",
        std::make_shared<std::vector<double>>(std::vector<double>{3, 4, 5})});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
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
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Text{{}, "a", std::make_shared<ValAdaptor>("zzz")});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
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
        Message::Text{{}, "a", std::make_shared<ValAdaptor>("zzzzz")});
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
TEST_F(ServerTest, robotModel) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::RobotModel{
        "a", std::make_shared<std::vector<RobotLink>>(
                 std::vector<RobotLink>{{"a", Geometry{}, ViewColor::black}})});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::RobotModel>{{}, "c1", "a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.commonLinks()->size(), 1);
        },
        [&] { ADD_FAILURE() << "RobotModel Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::RobotModel{
        "a", std::make_shared<std::vector<RobotLink>>(std::vector<RobotLink>{
                 RobotLink{"a", {}, Geometry{}, ViewColor::black},
                 RobotLink{"b", {}, Geometry{}, ViewColor::black},
                 RobotLink{"c",
                           {"j", "a", RobotJointType::fixed, {}, 0},
                           Geometry{},
                           ViewColor::black},
             })});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            ASSERT_EQ(obj.commonLinks()->size(), 3);
            EXPECT_EQ(obj.commonLinks()->at(0).joint.parent_name, "");
            EXPECT_EQ(obj.commonLinks()->at(1).joint.parent_name, "");
            EXPECT_EQ(obj.commonLinks()->at(2).joint.parent_name, "a");
        },
        [&] { ADD_FAILURE() << "RobotModel Res recv failed"; });
}
TEST_F(ServerTest, view) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::View{
        "a",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(
            std::unordered_map<std::string, Message::View::ViewComponent>{
                {"0", ViewComponents::text("a").toV().lockTmp(data_, "")},
                {"1", ViewComponents::newLine().lockTmp(data_, "")},
                {"2", ViewComponents::button(
                          "f", Func{Field{std::weak_ptr<Internal::ClientData>(),
                                          "p", "q"}})
                          .lockTmp(data_, "")}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
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
            EXPECT_EQ(obj.data_diff->at("0").type, ViewComponentType::text);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "View Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::View{
        "a",
        std::make_shared<
            std::unordered_map<std::string, Message::View::ViewComponent>>(
            std::unordered_map<std::string, Message::View::ViewComponent>{
                {"0", ViewComponents::text("b").toV().lockTmp(data_, "")},
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
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas3D{
        "a",
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas3D::Canvas3DComponent>>(
            std::unordered_map<std::string,
                               Message::Canvas3D::Canvas3DComponent>{
                {"0", {}}, {"1", {}}, {"2", {}}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::Canvas3D>{{}, "c1", "a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "Canvas3D Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas3D{
        "a",
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
            EXPECT_EQ(obj.sub_field, "");
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
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas2D{
        "a", 0, 0,
        std::make_shared<std::unordered_map<
            std::string, Message::Canvas2D::Canvas2DComponent>>(
            std::unordered_map<std::string,
                               Message::Canvas2D::Canvas2DComponent>{
                {"0", {}}, {"1", {}}, {"2", {}}}),
        3});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
    dummy_c2->send(Message::Req<Message::Canvas2D>{{}, "c1", "a", 1});
    wait();
    // req時の値
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.length, 3);
        },
        [&] { ADD_FAILURE() << "Canvas2D Res recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Sync{});
    dummy_c1->send(Message::Canvas2D{
        "a", 0, 0,
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
            EXPECT_EQ(obj.sub_field, "");
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
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    auto sendImage = [&] {
        dummy_c1->send(Message::Sync{});
        dummy_c1->send(Message::Image{
            "a", ImageFrame{10, 10,
                            std::make_shared<std::vector<unsigned char>>(
                                10 * 10 * 3)}});
    };
    sendImage();
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});

    // normal request
    dummy_c2->send(Message::Req<Message::Image>{"c1", "a", 1, {}});
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
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
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data().size(), 10 * 10 * 3);
            EXPECT_EQ(obj.rows(), 10);
            EXPECT_EQ(obj.cols(), 10);
            EXPECT_EQ(obj.color_mode(), ImageColorMode::bgr);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 1"; });
    dummy_c2->recvClear();

#if WEBCFACE_USE_OPENCV
    // resize, convert color, frame rate
    dummy_c2->send(Message::Req<Message::Image>{
        "c1",
        "a",
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
            EXPECT_EQ(obj.sub_field, "");
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
            EXPECT_EQ(obj.sub_field, "");
            EXPECT_EQ(obj.data().size(), 5 * 5 * 1);
            EXPECT_EQ(obj.rows(), 5);
            EXPECT_EQ(obj.cols(), 5);
            EXPECT_EQ(obj.color_mode(), ImageColorMode::gray);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 3"; });
    dummy_c2->recvClear();

    // compress
    dummy_c2->send(Message::Req<Message::Image>{
        "c1",
        "a",
        1,
        {std::nullopt, std::nullopt, std::nullopt, ImageCompressMode::png, 5,
         std::nullopt}});
    wait();
    wait();
    wait();
    dummy_c2->recv<Message::Sync>([&](auto) {},
                                  [&] { ADD_FAILURE() << "Sync recv failed"; });
    dummy_c2->recv<Message::Res<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(obj.sub_field, "");
            ImageFrame img{
                10, 10,
                std::make_shared<std::vector<unsigned char>>(10 * 10 * 3)};
            std::vector<unsigned char> dst;
            cv::imencode(".png", img.mat(), dst,
                         {cv::IMWRITE_PNG_COMPRESSION, 5});
            EXPECT_EQ(obj.data().size(), dst.size());
            EXPECT_EQ(obj.rows(), 10);
            EXPECT_EQ(obj.cols(), 10);
            EXPECT_EQ(obj.compress_mode(), ImageCompressMode::png);
        },
        [&] { ADD_FAILURE() << "Image Res recv failed 4"; });
    dummy_c2->recvClear();
#endif
}
TEST_F(ServerTest, log) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    Server::store.keep_log = 3;
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c1->send(Message::Log{
        0, std::make_shared<std::deque<Message::Log::LogLine>>(
               std::deque<Message::Log::LogLine>{
                   LogLine{0, std::chrono::system_clock::now(), "0"},
                   LogLine{1, std::chrono::system_clock::now(), "1"},
                   LogLine{2, std::chrono::system_clock::now(), "2"},
                   LogLine{3, std::chrono::system_clock::now(), "3"},
               })});
    wait();
    dummy_c2->send(Message::SyncInit{{}, "", 0, "", "", ""});
    dummy_c2->send(Message::LogReq{{}, "c1"});
    wait();
    // req時の値
    // keep_logを超えたので最後の3行だけ送られる
    dummy_c2->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.log->size(), 3);
            EXPECT_EQ(obj.log->at(0).level, 1);
            EXPECT_EQ(obj.log->at(0).message, "1");
        },
        [&] { ADD_FAILURE() << "Log recv failed"; });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(Message::Log{
        0, std::make_shared<std::deque<Message::Log::LogLine>>(
               std::deque<Message::Log::LogLine>{
                   LogLine{4, std::chrono::system_clock::now(), "4"},
                   LogLine{5, std::chrono::system_clock::now(), "5"},
                   LogLine{6, std::chrono::system_clock::now(), "6"},
                   LogLine{7, std::chrono::system_clock::now(), "7"},
                   LogLine{8, std::chrono::system_clock::now(), "8"},
               })});
    wait();
    dummy_c2->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.log->size(), 5);
            EXPECT_EQ(obj.log->at(0).level, 4);
            EXPECT_EQ(obj.log->at(0).message, "4");
        },
        [&] { ADD_FAILURE() << "Log recv failed"; });
}
TEST_F(ServerTest, call) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(Message::SyncInit{{}, "c1", 0, "", "", ""});
    dummy_c2->send(Message::SyncInit{{}, "c2", 0, "", "", ""});
    wait();
    // c2がc1にcallを送る (caller_id=1)
    dummy_c2->send(Message::Call{
        FuncCall{1, 0, 1, "a", {ValAdaptor(0), ValAdaptor(0), ValAdaptor(0)}}});
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
