#include "server_test.h"

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
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data->size(), 3u);
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
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data->size(), 4u);
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
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(*obj.data, "zzz");
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(
        message::Text{{}, "a"_ss, std::make_shared<ValAdaptor>("zzzzz")});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
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
        std::vector<std::shared_ptr<message::RobotLink>>{
            RobotLink{"a", Geometry{}, ViewColor::black}.lockJoints({})}});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::RobotModel>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::RobotModel>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data.size(), 1u);
    });
    dummy_c2->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::RobotModel{
        "a"_ss,
        std::vector<std::shared_ptr<message::RobotLink>>{
            RobotLink{"a", {}, Geometry{}, ViewColor::black}.lockJoints({}),
            RobotLink{"b", {}, Geometry{}, ViewColor::black}.lockJoints(
                {SharedString::fromU8String("a")}),
            RobotLink{"c",
                      {"j"_ss, "a"_ss, RobotJointType::fixed, {}, 0},
                      Geometry{},
                      ViewColor::black}
                .lockJoints({SharedString::fromU8String("a"),
                             SharedString::fromU8String("b")}),
        }});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::RobotModel>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        ASSERT_EQ(obj.data.size(), 3u);
        EXPECT_EQ(obj.data.at(0)->joint_parent, -1);
        EXPECT_EQ(obj.data.at(1)->joint_parent, -1);
        EXPECT_EQ(obj.data.at(2)->joint_parent, 0); // a
    });
}
TEST_F(ServerTest, view) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c3 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::View{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::ViewComponent>>{
            {"0", ViewComponents::text("a").component_v.lockTmp(data_, ""_ss)},
            {"1", ViewComponents::newLine().lockTmp(data_, ""_ss)},
            {"2", ViewComponents::button(
                      "f", Func{Field{std::weak_ptr<internal::ClientData>(),
                                      "p"_ss, "q"_ss}})
                      .lockTmp(data_, ""_ss)}},
        std::vector<SharedString>{"0"_ss, "1"_ss, "2"_ss}});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::View>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_ids->size(), 3u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
    });
    dummy_c2->recvClear();
    dummy_c3->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c3->send(message::Req<message::ViewOld>{{}, "c1"_ss, "a"_ss, 1});
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::ViewOld>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.length, 3u);
    });
    dummy_c3->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::View{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::ViewComponent>>{
            {"0", ViewComponents::text("b").component_v.lockTmp(data_, ""_ss)},
        },
        std::nullopt});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
    });
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::ViewOld>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.length, 3u);
    });
}
TEST_F(ServerTest, viewOld) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c3 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::ViewOld{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::ViewComponent>>{
            {"0", ViewComponents::text("a").component_v.lockTmp(data_, ""_ss)},
            {"1", ViewComponents::newLine().lockTmp(data_, ""_ss)},
            {"2", ViewComponents::button(
                      "f", Func{Field{std::weak_ptr<internal::ClientData>(),
                                      "p"_ss, "q"_ss}})
                      .lockTmp(data_, ""_ss)}},
        3});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::View>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_ids->size(), 3u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
    });
    dummy_c2->recvClear();
    dummy_c3->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c3->send(message::Req<message::ViewOld>{{}, "c1"_ss, "a"_ss, 1});
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::ViewOld>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.length, 3u);
    });
    dummy_c3->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::ViewOld{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::ViewComponent>>{
            {"0", ViewComponents::text("b").component_v.lockTmp(data_, ""_ss)},
        },
        3});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
    });
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::ViewOld>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff.at("0")->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.length, 3u);
    });
}
TEST_F(ServerTest, canvas3d) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c3 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas3D{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::Canvas3DComponent>>{
            {"0", {}}, {"1", {}}, {"2", {}}},
        std::vector<SharedString>{"0"_ss, "1"_ss, "2"_ss}});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Canvas3D>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_ids->size(), 3u);
    });
    dummy_c2->recvClear();
    dummy_c3->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c3->send(message::Req<message::Canvas3DOld>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas3DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 3u);
            EXPECT_EQ(obj.length, 3u);
        });
    dummy_c3->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas3D{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::Canvas3DComponent>>{
            {"0", {}},
        },
        std::nullopt});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_ids, std::nullopt);
    });
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas3DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 1u);
            EXPECT_EQ(obj.length, 3u);
        });
}
TEST_F(ServerTest, canvas3dOld) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c3 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas3DOld{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::Canvas3DComponent>>{
            {"0", {}}, {"1", {}}, {"2", {}}},
        3});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Canvas3D>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_ids->size(), 3u);
    });
    dummy_c2->recvClear();
    dummy_c3->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c3->send(message::Req<message::Canvas3DOld>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas3DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 3u);
            EXPECT_EQ(obj.length, 3u);
        });
    dummy_c3->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas3DOld{
        "a"_ss,
        std::map<std::string, std::shared_ptr<message::Canvas3DComponent>>{
            {"0", {}},
        },
        3});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_ids, std::nullopt);
    });
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas3DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 1u);
            EXPECT_EQ(obj.length, 3u);
        });
}
TEST_F(ServerTest, canvas2d) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c3 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas2D{
        "a"_ss, 0, 0,
        std::map<std::string, std::shared_ptr<message::Canvas2DComponent>>{
            {"0", {}}, {"1", {}}, {"2", {}}},
        std::vector<SharedString>{"0"_ss, "1"_ss, "2"_ss}});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Canvas2D>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_ids->size(), 3u);
    });
    dummy_c2->recvClear();
    dummy_c3->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c3->send(message::Req<message::Canvas2DOld>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas2DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 3u);
            EXPECT_EQ(obj.length, 3u);
        });
    dummy_c3->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas2D{
        "a"_ss, 0, 0,
        std::map<std::string, std::shared_ptr<message::Canvas2DComponent>>{
            {"0", {}},
        },
        std::nullopt});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_ids, std::nullopt);
    });
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas2DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 1u);
            EXPECT_EQ(obj.length, 3u);
        });
}
TEST_F(ServerTest, canvas2dOld) {
    dummy_c1 = std::make_shared<DummyClient>();
    dummy_c2 = std::make_shared<DummyClient>();
    dummy_c3 = std::make_shared<DummyClient>();
    dummy_c1->send(message::SyncInit{{}, "c1"_ss, 0, "", "", ""});
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas2DOld{
        "a"_ss, 0, 0,
        std::map<std::string, std::shared_ptr<message::Canvas2DComponent>>{
            {"0", {}}, {"1", {}}, {"2", {}}},
        3});
    wait();
    dummy_c2->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::Req<message::Canvas2D>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_ids->size(), 3u);
    });
    dummy_c2->recvClear();
    dummy_c3->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c3->send(message::Req<message::Canvas2DOld>{{}, "c1"_ss, "a"_ss, 1});
    // req時の値
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas2DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 3u);
            EXPECT_EQ(obj.length, 3u);
        });
    dummy_c3->recvClear();

    // 変化後の値
    dummy_c1->send(message::Sync{});
    dummy_c1->send(message::Canvas2DOld{
        "a"_ss, 0, 0,
        std::map<std::string, std::shared_ptr<message::Canvas2DComponent>>{
            {"0", {}},
        },
        3});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_ids, std::nullopt);
    });
    dummy_c3->waitRecv<message::Sync>([&](auto) {});
    dummy_c3->waitRecv<message::Res<message::Canvas2DOld>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.req_id, 1u);
            EXPECT_EQ(obj.sub_field.u8String(), "");
            EXPECT_EQ(obj.data_diff.size(), 1u);
            EXPECT_EQ(obj.length, 3u);
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
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_->size(), 15u * 10u * 3u);
        EXPECT_EQ(obj.width_, 15);
        EXPECT_EQ(obj.height_, 10);
        EXPECT_EQ(obj.color_mode_, ImageColorMode::bgr);
    });
    dummy_c2->recvClear();

    // 変化後の値
    sendImage();
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_->size(), 15u * 10u * 3u);
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
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_->size(), 8u * 5u * 1u);
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
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.data_->size(), 8u * 5u * 1u);
        EXPECT_EQ(obj.height_, 5);
        EXPECT_EQ(obj.width_, 8);
        EXPECT_EQ(obj.color_mode_, ImageColorMode::gray);
    });
    dummy_c2->recvClear();

    // compress
    dummy_c2->send(message::Req<message::Image>{
        "c1"_ss, "a"_ss, 1,
        message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                          ImageCompressMode::png, 5, std::nullopt}});
    dummy_c2->waitRecv<message::Sync>([&](auto) {});
    dummy_c2->waitRecv<message::Res<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj.sub_field.u8String(), "");
        EXPECT_EQ(obj.width_, 15);
        EXPECT_EQ(obj.height_, 10);
        EXPECT_GT(obj.data_->size(), 0u);
        EXPECT_EQ(obj.cmp_mode_, ImageCompressMode::png);
    });
    dummy_c2->recvClear();

    // convert color pattern
    for (int f_type = 0; f_type < 5; f_type++) {
        for (int t_type = 0; t_type < 5; t_type++) {
            dummy_c1->send(message::Sync{});
            dummy_c1->send(message::Image{
                SharedString::fromU8String("a" + std::to_string(f_type) +
                                           std::to_string(t_type)),
                ImageFrame{sizeWH(15, 10), static_cast<ImageColorMode>(f_type)}
                    .toMessage()});
            wait();
            dummy_c2->send(message::Req<message::Image>{
                "c1"_ss,
                SharedString::fromU8String("a" + std::to_string(f_type) +
                                           std::to_string(t_type)),
                1,
                message::ImageReq{std::nullopt, std::nullopt,
                                  static_cast<ImageColorMode>(t_type),
                                  ImageCompressMode::raw, 0, std::nullopt}});
            dummy_c2->waitRecv<message::Res<message::Image>>(
                [&](const auto &obj) {
                    EXPECT_EQ(obj.req_id, 1u);
                    EXPECT_EQ(obj.sub_field.u8String(), "");
                    EXPECT_EQ(obj.height_, 10);
                    EXPECT_EQ(obj.width_, 15);
                    EXPECT_EQ(obj.color_mode_,
                              static_cast<ImageColorMode>(t_type));
                });
            dummy_c2->recvClear();
        }
    }
}
