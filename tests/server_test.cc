#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/server/server.h>
#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include "webcface/message/message.h"
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
#include "webcface/image_frame.h"

using namespace webcface;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

static void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(encoding::castToU8(std::string_view(str, len)));
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
    std::shared_ptr<internal::ClientData> data_ =
        std::make_shared<internal::ClientData>("a"_ss);
    std::shared_ptr<DummyClient> dummy_c1, dummy_c2;
    int callback_called;
};

TEST_F(ServerTest, connection) {
    dummy_c1 = std::make_shared<DummyClient>();
    while (!dummy_c1->connected()) {
        wait();
    }
    dummy_c2 = std::make_shared<DummyClient>();
    while (!dummy_c2->connected()) {
        wait();
    }
    EXPECT_EQ(server->store->clients.size(), 2);
}
TEST_F(ServerTest, unixSocketConnection) {
    dummy_c1 = std::make_shared<DummyClient>(true);
    while (!dummy_c1->connected()) {
        wait();
    }
    dummy_c2 = std::make_shared<DummyClient>(true);
    while (!dummy_c2->connected()) {
        wait();
    }
    EXPECT_EQ(server->store->clients.size(), 2);
}
TEST_F(ServerTest, sync) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait(); // 接続順が変わるとmember idが変わってしまう
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::SyncInit{{}, "c2"_ss, 0, "a", "1", ""});
    dummy_c1->waitRecv<message::SyncInit>([&](const auto &obj) {
        EXPECT_EQ(obj.member_name, "c2"_ss);
        EXPECT_EQ(obj.member_id, 2);
        EXPECT_EQ(obj.lib_name, "a");
        EXPECT_EQ(obj.lib_ver, "1");
        EXPECT_EQ(obj.addr, "127.0.0.1");
    });
    dummy_c1->waitRecv<message::SvrVersion>([&](const auto &obj) {
        EXPECT_EQ(obj.svr_name, WEBCFACE_SERVER_NAME);
        EXPECT_EQ(obj.ver, WEBCFACE_VERSION);
    });
    ASSERT_TRUE(server->store->clients_by_id.count(1));
    ASSERT_TRUE(server->store->clients_by_id.count(2));
    EXPECT_EQ(server->store->clients_by_id.at(1)->name, ""_ss);
    EXPECT_EQ(server->store->clients_by_id.at(2)->name, "c2"_ss);
    dummy_c1->recvClear();

    // dummy_c2->send(message::Sync{});
    // wait();
}
TEST_F(ServerTest, ping) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    wait();
    auto start = std::chrono::steady_clock::now();
    server->server_ping_wait.notify_one(); // これで無理やりpingさせる
    auto s_c1 = server->store->clients_by_id.at(1);
    dummy_c1->waitRecv<message::Ping>([&](const auto &) {});
    dummy_c1->send(message::Ping{});
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
    dummy_c1->send(message::PingStatusReq{});
    dummy_c1->waitRecv<message::PingStatus>(
        [&](const auto &obj) { EXPECT_EQ(obj.status->size(), 0); });

    dummy_c1->recvClear();
    server->server_ping_wait.notify_one(); // これで無理やりpingさせる
    dummy_c1->waitRecv<message::PingStatus>([&](const auto &obj) {
        EXPECT_TRUE(obj.status->count(1));
        EXPECT_GE(obj.status->at(1), WEBCFACE_TEST_TIMEOUT - 1);
        EXPECT_LE(obj.status->at(1), dur_max);
    });
}
TEST_F(ServerTest, entry) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(
        message::Value{{}, "a"_ss, std::make_shared<std::vector<double>>(1)});
    dummy_c1->send(message::Text{{}, "a"_ss, std::make_shared<ValAdaptor>("")});
    dummy_c1->send(message::RobotModel{
        "a"_ss, std::make_shared<std::vector<message::RobotLink>>()});
    dummy_c1->send(message::Canvas3D{
        "a"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas3DComponent>>(),
        0});
    dummy_c1->send(message::Canvas2D{
        "a"_ss, 0, 0,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas2DComponent>>(),
        0});
    dummy_c1->send(message::View{
        "a"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::ViewComponent>>(),
        0});
    dummy_c1->send(message::Image{
        "a"_ss,
        ImageFrame{sizeWH(100, 100),
                   std::make_shared<std::vector<unsigned char>>(100 * 100 * 3),
                   ImageColorMode::bgr}
            .toMessage()});
    dummy_c1->send(
        message::FuncInfo{0, "a"_ss, ValType::none_,
                          std::make_shared<std::vector<message::Arg>>()});
    wait();
    // c2が接続したタイミングでのc1のentryが全部返る
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->waitRecv<message::SyncInit>([&](const auto &obj) {
        EXPECT_EQ(obj.member_name, "c1"_ss);
        EXPECT_EQ(obj.member_id, 1);
    });
    dummy_c2->waitRecv<message::Entry<message::Value>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
    });
    dummy_c2->waitRecv<message::Entry<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
    });
    dummy_c2->waitRecv<message::Entry<message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "a"_ss);
        });
    dummy_c2->waitRecv<message::Entry<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
    });
    dummy_c2->waitRecv<message::Entry<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
    });
    dummy_c2->waitRecv<message::Entry<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
    });
    dummy_c2->waitRecv<message::Entry<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
    });
    dummy_c2->waitRecv<message::FuncInfo>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.return_type, ValType::none_);
        EXPECT_EQ(obj.args->size(), 0);
    });
    dummy_c2->recvClear();

    // c1にentryを追加する
    dummy_c1->send(
        message::Value{{}, "b"_ss, std::make_shared<std::vector<double>>(1)});
    dummy_c2->waitRecv<message::Entry<message::Value>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    dummy_c1->send(message::Text{{}, "b"_ss, std::make_shared<ValAdaptor>("")});
    dummy_c2->waitRecv<message::Entry<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    dummy_c1->send(message::RobotModel{
        "b"_ss, std::make_shared<std::vector<message::RobotLink>>()});
    dummy_c2->waitRecv<message::Entry<message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 1);
            EXPECT_EQ(obj.field, "b"_ss);
        });
    dummy_c1->send(message::View{
        "b"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::ViewComponent>>(),
        0});
    dummy_c2->waitRecv<message::Entry<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    dummy_c1->send(message::Canvas3D{
        "b"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas3DComponent>>(),
        0});
    dummy_c2->waitRecv<message::Entry<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    dummy_c1->send(message::Canvas2D{
        "b"_ss, 0, 0,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas2DComponent>>(),
        0});
    dummy_c2->waitRecv<message::Entry<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    dummy_c1->send(message::Image{
        "b"_ss,
        ImageFrame{sizeWH(50, 50),
                   std::make_shared<std::vector<unsigned char>>(50 * 50 * 3),
                   ImageColorMode::bgr}
            .toMessage()});
    dummy_c2->waitRecv<message::Entry<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    dummy_c1->send(
        message::FuncInfo{0, "b"_ss, ValType::none_,
                          std::make_shared<std::vector<message::Arg>>()});
    dummy_c2->waitRecv<message::FuncInfo>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.return_type, ValType::none_);
        EXPECT_EQ(obj.args->size(), 0);
    });
}
TEST_F(ServerTest, value) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Value{
        {},
        "a"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{3, 4, 5})});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Value>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Value>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data->size(), 3);
        EXPECT_EQ(obj.data->at(0), 3);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Value{{},
                                  "a"_ss,
                                  std::make_shared<std::vector<double>>(
                                      std::vector<double>{6, 7, 8, 9})});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Value>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data->size(), 4);
        EXPECT_EQ(obj.data->at(0), 6);
    });
}
TEST_F(ServerTest, text) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(
        message::Text{{}, "a"_ss, std::make_shared<ValAdaptor>("zzz")});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Text>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(*obj.data, "zzz");
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(
        message::Text{{}, "a"_ss, std::make_shared<ValAdaptor>("zzzzz")});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(*obj.data, "zzzzz");
    });
}
TEST_F(ServerTest, robotModel) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::RobotModel{
        "a"_ss,
        std::make_shared<std::vector<message::RobotLink>>(
            std::vector<message::RobotLink>{
                RobotLink{"a", Geometry{}, ViewColor::black}.toMessage({})})});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::RobotModel>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::RobotModel>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data->size(), 1);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::RobotModel{
        "a"_ss,
        std::make_shared<std::vector<message::RobotLink>>(
            std::vector<message::RobotLink>{
                RobotLink{"a", {}, Geometry{}, ViewColor::black}.toMessage({}),
                RobotLink{"b", {}, Geometry{}, ViewColor::black}.toMessage(
                    {SharedString("a")}),
                RobotLink{"c",
                          {"j"_ss, "a"_ss, RobotJointType::fixed, {}, 0},
                          Geometry{},
                          ViewColor::black}
                    .toMessage({SharedString("a"), SharedString("b")}),
            })});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::RobotModel>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        ASSERT_EQ(obj.data->size(), 3);
        EXPECT_EQ(obj.data->at(0).joint_parent, -1);
        EXPECT_EQ(obj.data->at(1).joint_parent, -1);
        EXPECT_EQ(obj.data->at(2).joint_parent, 0); // a
    });
}
TEST_F(ServerTest, view) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::View{
        "a"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::ViewComponent>>(
            std::unordered_map<std::string, message::ViewComponent>{
                {"0", ViewComponents::text("a")
                          .toV()
                          .lockTmp(data_, ""_ss)
                          .toMessage()},
                {"1",
                 ViewComponents::newLine().lockTmp(data_, ""_ss).toMessage()},
                {"2", ViewComponents::button(
                          "f", Func{Field{std::weak_ptr<internal::ClientData>(),
                                          "p"_ss, "q"_ss}})
                          .lockTmp(data_, ""_ss)
                          .toMessage()}}),
        3});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::View>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_diff->size(), 3);
        EXPECT_EQ(obj.data_diff->at("0").type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.length, 3);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::View{
        "a"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::ViewComponent>>(
            std::unordered_map<std::string, message::ViewComponent>{
                {"0", ViewComponents::text("b")
                          .toV()
                          .lockTmp(data_, ""_ss)
                          .toMessage()},
            }),
        3});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_diff->size(), 1);
        EXPECT_EQ(obj.data_diff->at("0").type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.length, 3);
    });
}
TEST_F(ServerTest, canvas3d) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas3D{
        "a"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas3DComponent>>(
            std::unordered_map<std::string, message::Canvas3DComponent>{
                {"0", {}}, {"1", {}}, {"2", {}}}),
        3});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Canvas3D>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_diff->size(), 3);
        EXPECT_EQ(obj.length, 3);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas3D{
        "a"_ss,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas3DComponent>>(
            std::unordered_map<std::string, message::Canvas3DComponent>{
                {"0", {}},
            }),
        3});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_diff->size(), 1);
        EXPECT_EQ(obj.length, 3);
    });
}
TEST_F(ServerTest, canvas2d) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas2D{
        "a"_ss, 0, 0,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas2DComponent>>(
            std::unordered_map<std::string, message::Canvas2DComponent>{
                {"0", {}}, {"1", {}}, {"2", {}}}),
        3});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Canvas2D>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_diff->size(), 3);
        EXPECT_EQ(obj.length, 3);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas2D{
        "a"_ss, 0, 0,
        std::make_shared<
            std::unordered_map<std::string, message::Canvas2DComponent>>(
            std::unordered_map<std::string, message::Canvas2DComponent>{
                {"0", {}},
            }),
        3});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_diff->size(), 1);
        EXPECT_EQ(obj.length, 3);
    });
}
TEST_F(ServerTest, image) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    auto sendImage = [&] {
        dummy_c1->send(message::Sync{});
        dummy_c1->send(message::Image{
            "a"_ss,
            ImageFrame{sizeWH(15, 10), ImageColorMode::bgr}.toMessage()});
    };
    sendImage();
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});

    // normal request
    dummy_c2->send(message::Req<message::Image>{"c1"_ss, "a"_ss, 1, {}});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_->size(), 15 * 10 * 3);
        EXPECT_EQ(obj.width_, 15);
        EXPECT_EQ(obj.height_, 10);
        EXPECT_EQ(obj.color_mode_, ImageColorMode::bgr);
    });
    dummy_c2->recvClear();

    // 変化後の値
    sendImage();
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_->size(), 15 * 10 * 3);
        EXPECT_EQ(obj.height_, 10);
        EXPECT_EQ(obj.width_, 15);
        EXPECT_EQ(obj.color_mode_, ImageColorMode::bgr);
    });
    dummy_c2->recvClear();

    // resize, convert color, frame rate
    dummy_c2->send(message::Req<message::Image>{
        "c1"_ss, "a"_ss, 1,
        message::ImageReq{
            5, 8, ImageColorMode::gray, ImageCompressMode::raw, 0,
            1000.0 / WEBCFACE_TEST_TIMEOUT /
                3 // wait()のtimeoutに間に合わないようにする
        }});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_->size(), 8 * 5 * 1);
        EXPECT_EQ(obj.height_, 5);
        EXPECT_EQ(obj.width_, 8);
        EXPECT_EQ(obj.color_mode_, ImageColorMode::gray);
    });
    dummy_c2->recvClear();

    sendImage();
    wait();
    dummy_c2->recv<message::Res<message::Image>>(
        [&](auto) { ADD_FAILURE() << "should not receive Image Res 3"; },
        [] {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.data_->size(), 8 * 5 * 1);
        EXPECT_EQ(obj.height_, 5);
        EXPECT_EQ(obj.width_, 8);
        EXPECT_EQ(obj.color_mode_, ImageColorMode::gray);
    });
    dummy_c2->recvClear();

    // compress
    dummy_c2->send(message::Req<message::Image>{
        "c1"_ss, "a"_ss, 1,
        message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                          static_cast<int>(ImageCompressMode::png), 5,
                          std::nullopt}});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj.sub_field, ""_ss);
        EXPECT_EQ(obj.width_, 15);
        EXPECT_EQ(obj.height_, 10);
        EXPECT_GT(obj.data_->size(), 0);
        EXPECT_EQ(obj.cmp_mode_, ImageCompressMode::png);
    });
    dummy_c2->recvClear();

    // convert color pattern
    for (int f_type = 0; f_type < 5; f_type++) {
        for (int t_type = 0; t_type < 5; t_type++) {
            dummy_c1->send(message::Sync{});
            dummy_c1->send(message::Image{
                SharedString(encoding::castToU8("a" + std::to_string(f_type) +
                                                std::to_string(t_type))),
                ImageFrame{sizeWH(15, 10), static_cast<ImageColorMode>(f_type)}
                    .toMessage()});
            wait();
            dummy_c2->send(message::Req<message::Image>{
                "c1"_ss,
                SharedString(encoding::castToU8("a" + std::to_string(f_type) +
                                                std::to_string(t_type))),
                1,
                message::ImageReq{std::nullopt, std::nullopt, t_type,
                                  ImageCompressMode::raw, 0,
                                  std::nullopt}});
            dummy_c2->waitRecv<message::Res<message::Image>>(
                [&](const auto &obj) {
                    EXPECT_EQ(obj.req_id, 1);
                    EXPECT_EQ(obj.sub_field, ""_ss);
                    EXPECT_EQ(obj.height_, 10);
                    EXPECT_EQ(obj.width_, 15);
                    EXPECT_EQ(obj.color_mode_, static_cast<ImageColorMode>(t_type));
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
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Log{
        0, std::make_shared<std::deque<message::LogLine>>(
               std::deque<message::LogLine>{
                   LogLineData<>{0, std::chrono::system_clock::now(), "0"_ss}
                       .toMessage(),
                   LogLineData<>{1, std::chrono::system_clock::now(), "1"_ss}
                       .toMessage(),
                   LogLineData<>{2, std::chrono::system_clock::now(), "2"_ss}
                       .toMessage(),
                   LogLineData<>{3, std::chrono::system_clock::now(), "3"_ss}
                       .toMessage(),
               })});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::LogReq{{}, "c1"_ss});
    // req時の値
    // keep_logを超えたので最後の3行だけ送られる
    dummy_c2->waitRecv<message::Log>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.log->size(), 3);
        EXPECT_EQ(obj.log->at(0).level_, 1);
        EXPECT_EQ(obj.log->at(0).message_, "1"_ss);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Log{
        0, std::make_shared<std::deque<message::LogLine>>(
               std::deque<message::LogLine>{
                   LogLineData<>{4, std::chrono::system_clock::now(), "4"_ss}
                       .toMessage(),
                   LogLineData<>{5, std::chrono::system_clock::now(), "5"_ss}
                       .toMessage(),
                   LogLineData<>{6, std::chrono::system_clock::now(), "6"_ss}
                       .toMessage(),
                   LogLineData<>{7, std::chrono::system_clock::now(), "7"_ss}
                       .toMessage(),
                   LogLineData<>{8, std::chrono::system_clock::now(), "8"_ss}
                       .toMessage(),
               })});
    dummy_c2->waitRecv<message::Log>([&](const auto &obj) {
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_EQ(obj.log->size(), 5);
        EXPECT_EQ(obj.log->at(0).level_, 4);
        EXPECT_EQ(obj.log->at(0).message_, "4"_ss);
    });
}
TEST_F(ServerTest, call) {
    dummy_c1 = std::make_shared<DummyClient>();
    wait();
    dummy_c2 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c2->send(message::SyncInit{{}, "c2"_ss, 0, "", "", ""});
    wait();
    // c2がc1にcallを送る (caller_id=1)
    dummy_c2->send(message::Call{
        1, 0, 1, "a"_ss, {ValAdaptor(0), ValAdaptor(0), ValAdaptor(0)}});
    dummy_c1->waitRecv<message::Call>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 1);
        EXPECT_EQ(obj.caller_member_id, 2);
        EXPECT_EQ(obj.target_member_id, 1);
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.args.size(), 3);
    });
    dummy_c2->recvClear();

    dummy_c1->send(message::CallResponse{{}, 1, 2, true});
    dummy_c2->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 1);
        EXPECT_EQ(obj.caller_member_id, 2);
        EXPECT_EQ(obj.started, true);
    });
    dummy_c2->recvClear();

    dummy_c1->send(message::CallResult{{}, 1, 2, false, ValAdaptor("aaa")});
    dummy_c2->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 1);
        EXPECT_EQ(obj.caller_member_id, 2);
        EXPECT_EQ(obj.is_error, false);
        EXPECT_EQ(static_cast<std::string>(obj.result), "aaa");
    });
}
