#include "client_test.h"

TEST_F(ClientTest, canvas2DSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->canvas2d_store.setSend(
        "a"_ss, std::make_shared<Canvas2DDataBase>(
                    100, 100,
                    std::vector<Canvas2DComponent>{
                        geometries::line({0, 0}, {30, 30})
                            .color(ViewColor::black)
                            .fillColor(ViewColor::white)
                            .strokeWidth(5)
                            .onClick(Func{Field{data_, self_name, "f"_ss}})
                            .to2()
                            .lockTmp(data_, ""_ss, nullptr),
                        geometries::rect({0, 0}, {30, 30})
                            .color(ViewColor::black)
                            .fillColor(ViewColor::white)
                            .strokeWidth(5)
                            .onClick(Func{Field{data_, self_name, "f"_ss}})
                            .to2()
                            .lockTmp(data_, ""_ss, nullptr),
                        geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                            .color(ViewColor::black)
                            .fillColor(ViewColor::white)
                            .strokeWidth(5)
                            .onClick(Func{Field{data_, self_name, "f"_ss}})
                            .to2()
                            .lockTmp(data_, ""_ss, nullptr),
                    }));
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas2D>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.length, 3u);
        ASSERT_EQ(obj.data_diff->size(), 3u);
        EXPECT_EQ(obj.width, 100);
        EXPECT_EQ(obj.height, 100);
        EXPECT_EQ((*obj.data_diff)["0"].type,
                  static_cast<int>(Canvas2DComponentType::geometry));
        EXPECT_EQ((*obj.data_diff)["0"].color,
                  static_cast<int>(ViewColor::black));
        EXPECT_EQ((*obj.data_diff)["0"].fill,
                  static_cast<int>(ViewColor::white));
        EXPECT_EQ((*obj.data_diff)["0"].properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 0}));
        EXPECT_EQ((*obj.data_diff)["0"].geometry_type,
                  static_cast<int>(GeometryType::line));
        EXPECT_EQ((*obj.data_diff)["0"].on_click_member, self_name);
        EXPECT_EQ((*obj.data_diff)["0"].on_click_field, "f"_ss);
        EXPECT_EQ((*obj.data_diff)["1"].geometry_type,
                  static_cast<int>(GeometryType::rect));
        EXPECT_EQ((*obj.data_diff)["2"].geometry_type,
                  static_cast<int>(GeometryType::polygon));
    });
    dummy_s->recvClear();

    data_->canvas2d_store.setSend(
        "a"_ss, std::make_shared<Canvas2DDataBase>(
                    100, 100,
                    std::vector<Canvas2DComponent>{
                        geometries::line({0, 0}, {30, 30})
                            .color(ViewColor::red) // changed
                            .fillColor(ViewColor::white)
                            .strokeWidth(5)
                            .onClick(Func{Field{data_, self_name, "f"_ss}})
                            .to2()
                            .lockTmp(data_, ""_ss, nullptr),
                        geometries::rect({0, 0}, {30, 30})
                            .color(ViewColor::black)
                            .fillColor(ViewColor::white)
                            .strokeWidth(5)
                            .onClick(Func{Field{data_, self_name, "f"_ss}})
                            .to2()
                            .lockTmp(data_, ""_ss, nullptr),
                        geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                            .color(ViewColor::black)
                            .fillColor(ViewColor::white)
                            .strokeWidth(5)
                            .onClick(Func{Field{data_, self_name, "f"_ss}})
                            .to2()
                            .lockTmp(data_, ""_ss, nullptr),
                    }));
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas2D>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.length, 3u);
        ASSERT_EQ(obj.data_diff->size(), 1u);
        EXPECT_EQ((*obj.data_diff)["0"].type,
                  static_cast<int>(Canvas2DComponentType::geometry));
        EXPECT_EQ((*obj.data_diff)["0"].color,
                  static_cast<int>(ViewColor::red));
        EXPECT_EQ((*obj.data_diff)["0"].fill,
                  static_cast<int>(ViewColor::white));
        EXPECT_EQ((*obj.data_diff)["0"].properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 0}));
    });
}
TEST_F(ClientTest, canvas2DReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").canvas2D("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Canvas2D>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    wcli_->member("a").canvas2D("b").onChange(callback<Canvas2D>());

    auto v = std::make_shared<
        std::unordered_map<std::string, message::Canvas2DComponent>>(
        std::unordered_map<std::string, message::Canvas2DComponent>{
            {"0", geometries::line({0, 0}, {30, 30})
                      .color(ViewColor::black)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"_ss}})
                      .to2()
                      .lockTmp(data_, ""_ss, nullptr)
                      .toMessage()},
            {"1", geometries::rect({0, 0}, {30, 30})
                      .color(ViewColor::black)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"_ss}})
                      .to2()
                      .lockTmp(data_, ""_ss, nullptr)
                      .toMessage()},
            {"2", geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                      .color(ViewColor::black)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"_ss}})
                      .to2()
                      .lockTmp(data_, ""_ss, nullptr)
                      .toMessage()},
        });
    dummy_s->send(message::Res<message::Canvas2D>{1, ""_ss, 200, 200, v, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Canvas2D>{1, "c"_ss, 200, 200, v, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas2d_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss).value()->width,
              200);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss).value()->height,
              200);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3u);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .type(),
              Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .color(),
              ViewColor::black);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .fillColor(),
              ViewColor::white);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .geometry()
                  ->type,
              GeometryType::line);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .geometry()
                  ->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 0}));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .onClick()
                  ->member()
                  .name(),
              self_name.decode());
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .onClick()
                  ->name(),
              "f");
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(1)
                  .type(),
              Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(1)
                  .geometry()
                  ->type,
              GeometryType::rect);
    EXPECT_TRUE(data_->canvas2d_store.getRecv("a"_ss, "b.c"_ss).has_value());

    // 差分だけ送る
    auto v2 = std::make_shared<
        std::unordered_map<std::string, message::Canvas2DComponent>>(
        std::unordered_map<std::string, message::Canvas2DComponent>{
            {"0", geometries::line({0, 0}, {30, 30})
                      .color(ViewColor::red)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"_ss}})
                      .to2()
                      .lockTmp(data_, ""_ss, nullptr)
                      .toMessage()},
        });
    dummy_s->send(message::Res<message::Canvas2D>{1, ""_ss, 100, 100, v2, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3u);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .type(),
              Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .color(),
              ViewColor::red);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .fillColor(),
              ViewColor::white);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .geometry()
                  ->type,
              GeometryType::line);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(0)
                  .geometry()
                  ->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 0}));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(1)
                  .type(),
              Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at(1)
                  .geometry()
                  ->type,
              GeometryType::rect);
}
TEST_F(ClientTest, canvas3DSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->canvas3d_store.setSend(
        "a"_ss,
        std::make_shared<std::vector<Canvas3DComponent>>(
            std::vector<Canvas3DComponent>{
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(
                     geometries::line({0, 0, 0}, {30, 30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(
                     geometries::rect({0, 0}, {30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(geometries::sphere({0, 0, 0}, 1)),
                 std::nullopt,
                 {}},
            }));
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas3D>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.length, 3u);
        ASSERT_EQ(obj.data_diff->size(), 3u);
        EXPECT_EQ((*obj.data_diff)["0"].type,
                  static_cast<int>(Canvas3DComponentType::geometry));
        EXPECT_EQ((*obj.data_diff)["0"].color,
                  static_cast<int>(ViewColor::black));
        EXPECT_EQ((*obj.data_diff)["0"].geometry_properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 30}));
        EXPECT_EQ((*obj.data_diff)["0"].geometry_type,
                  static_cast<int>(GeometryType::line));
        EXPECT_EQ((*obj.data_diff)["1"].geometry_type,
                  static_cast<int>(GeometryType::rect));
        EXPECT_EQ((*obj.data_diff)["2"].geometry_type,
                  static_cast<int>(GeometryType::sphere));
    });
    dummy_s->recvClear();

    data_->canvas3d_store.setSend(
        "a"_ss,
        std::make_shared<std::vector<Canvas3DComponent>>(
            std::vector<Canvas3DComponent>{
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::red,
                 std::make_optional<Geometry>(
                     geometries::line({0, 0, 0}, {30, 30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(
                     geometries::rect({0, 0}, {30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(geometries::sphere({0, 0, 0}, 1)),
                 std::nullopt,
                 {}},
            }));
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas3D>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.length, 3u);
        ASSERT_EQ(obj.data_diff->size(), 1u);
        EXPECT_EQ((*obj.data_diff)["0"].type,
                  static_cast<int>(Canvas3DComponentType::geometry));
        EXPECT_EQ((*obj.data_diff)["0"].color,
                  static_cast<int>(ViewColor::red));
        EXPECT_EQ((*obj.data_diff)["0"].geometry_properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 30}));
        EXPECT_EQ((*obj.data_diff)["0"].geometry_type,
                  static_cast<int>(GeometryType::line));
    });
}
TEST_F(ClientTest, canvas3DReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").canvas3D("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Canvas3D>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    wcli_->member("a").canvas3D("b").onChange(callback<Canvas3D>());

    auto v = std::make_shared<
        std::unordered_map<std::string, message::Canvas3DComponent>>(
        std::unordered_map<std::string, message::Canvas3DComponent>{
            {"0",
             Canvas3DComponent{Canvas3DComponentType::geometry,
                               identity(),
                               ViewColor::black,
                               std::make_optional<Geometry>(
                                   geometries::line({0, 0, 0}, {30, 30, 30})),
                               std::nullopt,
                               {}}
                 .toMessage()},
            {"1", Canvas3DComponent{Canvas3DComponentType::geometry,
                                    identity(),
                                    ViewColor::black,
                                    std::make_optional<Geometry>(
                                        geometries::rect({0, 0}, {30, 30})),
                                    std::nullopt,
                                    {}}
                      .toMessage()},
            {"2", Canvas3DComponent{Canvas3DComponentType::geometry,
                                    identity(),
                                    ViewColor::black,
                                    std::make_optional<Geometry>(
                                        geometries::sphere({0, 0, 0}, 1)),
                                    std::nullopt,
                                    {}}
                      .toMessage()},
        });
    dummy_s->send(message::Res<message::Canvas3D>{1, ""_ss, v, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Canvas3D>{1, "c"_ss, v, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->size(), 3u);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->at(0).type(),
        Canvas3DComponentType::geometry);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->at(0).color(),
        ViewColor::black);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->at(0)
                  .geometry()
                  ->type,
              GeometryType::line);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->at(0)
                  .geometry()
                  ->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 30}));
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->at(1).type(),
        Canvas3DComponentType::geometry);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->at(1)
                  .geometry()
                  ->type,
              GeometryType::rect);
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a"_ss, "b.c"_ss).has_value());

    // 差分だけ送る
    auto v2 = std::make_shared<
        std::unordered_map<std::string, message::Canvas3DComponent>>(
        std::unordered_map<std::string, message::Canvas3DComponent>{
            {"0",
             Canvas3DComponent{Canvas3DComponentType::geometry,
                               identity(),
                               ViewColor::red,
                               std::make_optional<Geometry>(
                                   geometries::line({0, 0, 0}, {30, 30, 30})),
                               std::nullopt,
                               {}}
                 .toMessage()},
        });
    dummy_s->send(message::Res<message::Canvas3D>{1, ""_ss, v2, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->size(), 3u);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->at(0).type(),
        Canvas3DComponentType::geometry);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->at(0).color(),
        ViewColor::red);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->at(0)
                  .geometry()
                  ->type,
              GeometryType::line);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->at(0)
                  .geometry()
                  ->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 30}));
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->at(1).type(),
        Canvas3DComponentType::geometry);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->at(1)
                  .geometry()
                  ->type,
              GeometryType::rect);
}
TEST_F(ClientTest, robotModelSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->robot_model_store.setSend(
        "a"_ss, std::make_shared<std::vector<RobotLink>>(std::vector<RobotLink>{
                    {"a", Geometry{}, ViewColor::black}}));
    wcli_->sync();
    dummy_s->waitRecv<message::RobotModel>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data->size(), 1u);
    });
}
TEST_F(ClientTest, robotModelReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").robotModel("b").tryGet();
    dummy_s->waitRecv<message::Req<message::RobotModel>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    wcli_->member("a").robotModel("b").onChange(callback<RobotModel>());
    dummy_s->send(message::Res<message::RobotModel>(
        1, ""_ss,
        std::make_shared<std::vector<message::RobotLink>>(
            std::vector<message::RobotLink>{
                RobotLink{"a", Geometry{}, ViewColor::black}.toMessage({})})));
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::RobotModel>(
        1, "c"_ss,
        std::make_shared<std::vector<message::RobotLink>>(
            std::vector<message::RobotLink>{
                RobotLink{"a", Geometry{}, ViewColor::black}.toMessage({})})));
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->robot_model_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->robot_model_store.getRecv("a"_ss, "b"_ss).value()->size(),
              1u);
    EXPECT_TRUE(data_->robot_model_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(
        data_->robot_model_store.getRecv("a"_ss, "b.c"_ss).value()->size(), 1u);
}
TEST_F(ClientTest, imageSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->image_store.setSend(
        "a"_ss,
        ImageFrame{sizeWH(100, 100),
                   std::make_shared<std::vector<unsigned char>>(100 * 100 * 3),
                   ImageColorMode::bgr});
    wcli_->sync();
    dummy_s->waitRecv<message::Image>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data_->size(), 100u * 100u * 3u);
    });
}
TEST_F(ClientTest, imageReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").image("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Image>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
        EXPECT_EQ(obj,
                  (message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                                     ImageCompressMode::raw, 0, std::nullopt}));
    });
    wcli_->member("a").image("b").onChange(callback<Image>());
    ImageFrame img(sizeWH(100, 100),
                   std::make_shared<std::vector<unsigned char>>(100 * 100 * 3),
                   ImageColorMode::bgr);
    dummy_s->send(message::Res<message::Image>{1, ""_ss, img.toMessage()});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Image>{1, "c"_ss, img.toMessage()});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    ASSERT_TRUE(data_->image_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->image_store.getRecv("a"_ss, "b"_ss)->data().size(),
              img.data().size());
    ASSERT_TRUE(data_->image_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(data_->image_store.getRecv("a"_ss, "b.c"_ss)->data().size(),
              img.data().size());
}
