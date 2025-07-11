#include "test_common.h"
#include "client_test.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/robot_model.h"
#include "webcface/internal/robot_link_internal.h"
#include "webcface/internal/component_internal.h"

TEST_F(ClientTest, canvas2DSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto canvas_data = std::make_shared<message::Canvas2DData>(100, 100);
    canvas_data->components = {
        {"a", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::line({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
        {"b", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::rect({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
        {"c", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
    };
    canvas_data->data_ids = {"a"_ss, "b"_ss, "c"_ss};
    data_->canvas2d_store.setSend("a"_ss, canvas_data);
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas2D>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_NE(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_ids->size(), 3u);
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.width, 100);
        EXPECT_EQ(obj.height, 100);
        EXPECT_EQ(obj.data_diff["a"]->type,
                  static_cast<int>(Canvas2DComponentType::geometry));
        EXPECT_EQ(obj.data_diff["a"]->color,
                  static_cast<int>(ViewColor::black));
        EXPECT_EQ(obj.data_diff["a"]->fill, static_cast<int>(ViewColor::white));
        EXPECT_EQ(obj.data_diff["a"]->properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 0}));
        EXPECT_EQ(obj.data_diff["a"]->geometry_type,
                  static_cast<int>(GeometryType::line));
        EXPECT_EQ(obj.data_diff["a"]->on_click_member, self_name);
        EXPECT_EQ(obj.data_diff["a"]->on_click_field, "f"_ss);
        EXPECT_EQ(obj.data_diff["b"]->geometry_type,
                  static_cast<int>(GeometryType::rect));
        EXPECT_EQ(obj.data_diff["c"]->geometry_type,
                  static_cast<int>(GeometryType::polygon));
    });
    dummy_s->recvClear();

    canvas_data = std::make_shared<message::Canvas2DData>(*canvas_data);
    canvas_data->components = {
        {"a", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::line({0, 0}, {30, 30})
                          .color(ViewColor::red) // changed
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
        {"b", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::rect({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
        {"c", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
    };
    data_->canvas2d_store.setSend("a"_ss, canvas_data);
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas2D>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff["a"]->type,
                  static_cast<int>(Canvas2DComponentType::geometry));
        EXPECT_EQ(obj.data_diff["a"]->color, static_cast<int>(ViewColor::red));
        EXPECT_EQ(obj.data_diff["a"]->fill, static_cast<int>(ViewColor::white));
        EXPECT_EQ(obj.data_diff["a"]->properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 0}));
    });
    dummy_s->recvClear();

    canvas_data = std::make_shared<message::Canvas2DData>(*canvas_data);
    canvas_data->data_ids = {"a"_ss, "b"_ss};
    data_->canvas2d_store.setSend("a"_ss, canvas_data);
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas2D>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_NE(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_ids->size(), 2u);
        EXPECT_EQ(obj.data_diff.size(), 0u);
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

    std::map<std::string, std::shared_ptr<message::Canvas2DComponentData>> v{
        {"0", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::line({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
        {"1", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::rect({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
        {"2", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                          .color(ViewColor::black)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
    };
    std::vector<SharedString> v_ids = {"0"_ss, "1"_ss, "2"_ss};
    dummy_s->send(
        message::Res<message::Canvas2D>{1, ""_ss, 200, 200, v, v_ids});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(
        message::Res<message::Canvas2D>{1, "c"_ss, 200, 200, v, v_ids});
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
                  ->components.at("0")
                  ->type,
              static_cast<int>(Canvas2DComponentType::geometry));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->color,
              static_cast<int>(ViewColor::black));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->fill,
              static_cast<int>(ViewColor::white));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->geometry_type,
              static_cast<int>(GeometryType::line));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 0}));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->on_click_member->u8String(),
              self_name.decode());
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->on_click_field->u8String(),
              "f");
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->type,
              static_cast<int>(Canvas2DComponentType::geometry));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->geometry_type,
              static_cast<int>(GeometryType::rect));
    EXPECT_TRUE(data_->canvas2d_store.getRecv("a"_ss, "b.c"_ss).has_value());

    // 差分だけ送る
    std::map<std::string, std::shared_ptr<message::Canvas2DComponentData>> v2{
        {"0", std::static_pointer_cast<message::Canvas2DComponentData>(
                  std::shared_ptr(
                      geometries::line({0, 0}, {30, 30})
                          .color(ViewColor::red)
                          .fillColor(ViewColor::white)
                          .strokeWidth(5)
                          .onClick(Func{Field{data_, self_name, "f"_ss}})
                          .component_2d.lockTmp(data_, ""_ss, nullptr)))},
    };
    dummy_s->send(
        message::Res<message::Canvas2D>{1, ""_ss, 100, 100, v2, std::nullopt});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3u);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->type,
              static_cast<int>(Canvas2DComponentType::geometry));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->color,
              static_cast<int>(ViewColor::red));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->fill,
              static_cast<int>(ViewColor::white));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->geometry_type,
              static_cast<int>(GeometryType::line));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 0}));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->type,
              static_cast<int>(Canvas2DComponentType::geometry));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->geometry_type,
              static_cast<int>(GeometryType::rect));
}
TEST_F(ClientTest, canvas3DSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto canvas_data = std::make_shared<message::Canvas3DData>();
    canvas_data->components = {
        {"a", std::static_pointer_cast<message::Canvas3DComponentData>(
                  std::shared_ptr(
                      geometries::line({0, 0, 0}, {30, 30, 30})
                          .color(ViewColor::black)
                          .component_3d.lockTmp(data_, ""_ss, nullptr)))},
        {"b", std::static_pointer_cast<message::Canvas3DComponentData>(
                  std::shared_ptr(
                      geometries::rect({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .component_3d.lockTmp(data_, ""_ss, nullptr)))},
        {"c", std::static_pointer_cast<message::Canvas3DComponentData>(
                  std::shared_ptr(
                      geometries::sphere({0, 0, 0}, 1)
                          .color(ViewColor::black)
                          .component_3d.lockTmp(data_, ""_ss, nullptr)))},
    };
    canvas_data->data_ids = {"a"_ss, "b"_ss, "c"_ss};
    data_->canvas3d_store.setSend("a"_ss, canvas_data);
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas3D>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_NE(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_ids->size(), 3u);
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_diff["a"]->type,
                  static_cast<int>(Canvas3DComponentType::geometry));
        EXPECT_EQ(obj.data_diff["a"]->color,
                  static_cast<int>(ViewColor::black));
        EXPECT_EQ(obj.data_diff["a"]->geometry_properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 30}));
        EXPECT_EQ(obj.data_diff["a"]->geometry_type,
                  static_cast<int>(GeometryType::line));
        EXPECT_EQ(obj.data_diff["b"]->geometry_type,
                  static_cast<int>(GeometryType::rect));
        EXPECT_EQ(obj.data_diff["c"]->geometry_type,
                  static_cast<int>(GeometryType::sphere));
    });
    dummy_s->recvClear();

    canvas_data = std::make_shared<message::Canvas3DData>(*canvas_data);
    canvas_data->components = {
        {"a", std::static_pointer_cast<message::Canvas3DComponentData>(
                  std::shared_ptr(
                      geometries::line({0, 0, 0}, {30, 30, 30})
                          .color(ViewColor::red)
                          .component_3d.lockTmp(data_, ""_ss, nullptr)))},
        {"b", std::static_pointer_cast<message::Canvas3DComponentData>(
                  std::shared_ptr(
                      geometries::rect({0, 0}, {30, 30})
                          .color(ViewColor::black)
                          .component_3d.lockTmp(data_, ""_ss, nullptr)))},
        {"c", std::static_pointer_cast<message::Canvas3DComponentData>(
                  std::shared_ptr(
                      geometries::sphere({0, 0, 0}, 1)
                          .color(ViewColor::black)
                          .component_3d.lockTmp(data_, ""_ss, nullptr)))},
    };
    data_->canvas3d_store.setSend("a"_ss, canvas_data);
    wcli_->sync();
    dummy_s->waitRecv<message::Canvas3D>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data_ids, std::nullopt);
        ASSERT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff["a"]->type,
                  static_cast<int>(Canvas3DComponentType::geometry));
        EXPECT_EQ(obj.data_diff["a"]->color, static_cast<int>(ViewColor::red));
        EXPECT_EQ(obj.data_diff["a"]->geometry_properties,
                  (std::vector<double>{0, 0, 0, 30, 30, 30}));
        EXPECT_EQ(obj.data_diff["a"]->geometry_type,
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

    std::map<std::string, std::shared_ptr<message::Canvas3DComponentData>> v{
        {
            "0",
            std::static_pointer_cast<message::Canvas3DComponentData>(
                std::shared_ptr(
                    geometries::line({0, 0, 0}, {30, 30, 30})
                        .color(ViewColor::black)
                        .component_3d.lockTmp(data_, ""_ss, nullptr))),
        },
        {
            "1",
            std::static_pointer_cast<message::Canvas3DComponentData>(
                std::shared_ptr(
                    geometries::rect({0, 0}, {30, 30})
                        .color(ViewColor::black)
                        .component_3d.lockTmp(data_, ""_ss, nullptr))),
        },
        {
            "2",
            std::static_pointer_cast<message::Canvas3DComponentData>(
                std::shared_ptr(
                    geometries::sphere({0, 0, 0}, 1)
                        .color(ViewColor::black)
                        .component_3d.lockTmp(data_, ""_ss, nullptr))),
        },
    };
    std::vector<SharedString> v_ids = {"0"_ss, "1"_ss, "2"_ss};
    dummy_s->send(message::Res<message::Canvas3D>{1, ""_ss, v, v_ids});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Canvas3D>{1, "c"_ss, v, v_ids});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3u);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->type,
              static_cast<int>(Canvas3DComponentType::geometry));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->color,
              static_cast<int>(ViewColor::black));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->geometry_type,
              static_cast<int>(GeometryType::line));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->geometry_properties,
              (std::vector<double>{0, 0, 0, 30, 30, 30}));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->type,
              static_cast<int>(Canvas3DComponentType::geometry));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->geometry_type,
              static_cast<int>(GeometryType::rect));
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a"_ss, "b.c"_ss).has_value());

    // 差分だけ送る
    std::map<std::string, std::shared_ptr<message::Canvas3DComponentData>> v2{
        {
            "0",
            std::static_pointer_cast<message::Canvas3DComponentData>(
                std::shared_ptr(
                    geometries::line({0, 0, 0}, {30, 30, 30})
                        .color(ViewColor::red)
                        .component_3d.lockTmp(data_, ""_ss, nullptr))),
        },
    };
    dummy_s->send(message::Res<message::Canvas3D>{1, ""_ss, v2, std::nullopt});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3u);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->type,
              static_cast<int>(Canvas3DComponentType::geometry));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->color,
              static_cast<int>(ViewColor::red));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->geometry_type,
              static_cast<int>(GeometryType::line));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("0")
                  ->geometry_properties,
              (std::vector<double>{0, 0, 0, 30, 30, 30}));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->type,
              static_cast<int>(Canvas3DComponentType::geometry));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.at("1")
                  ->geometry_type,
              static_cast<int>(GeometryType::rect));
}
TEST_F(ClientTest, robotModelSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto ln = std::make_shared<internal::RobotLinkData>();
    ln->name = SharedString::fromU8String("a");
    data_->robot_model_store.setSend(
        "a"_ss,
        std::make_shared<std::vector<std::shared_ptr<internal::RobotLinkData>>>(
            std::vector<std::shared_ptr<internal::RobotLinkData>>{ln}));
    wcli_->sync();
    dummy_s->waitRecv<message::RobotModel>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data.size(), 1u);
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
        std::vector<std::shared_ptr<message::RobotLink>>{
            RobotLink{"a", Geometry{}, ViewColor::black}.lockJoints({})}));
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::RobotModel>(
        1, "c"_ss,
        std::vector<std::shared_ptr<message::RobotLink>>{
            RobotLink{"a", Geometry{}, ViewColor::black}.lockJoints({})}));
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
        EXPECT_EQ(obj, (message::ImageReq{
                           std::nullopt, std::nullopt, std::nullopt,
                           message::ImageCompressMode::raw, 0, std::nullopt}));
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
