#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/client.h>
#include <webcface/logger.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/image.h>
#include <webcface/canvas3d.h>
#include <webcface/canvas2d.h>
#include <webcface/robot_model.h>
#include <webcface/common/def.h>
#include "../message/message.h"
#include <chrono>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace webcface;

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
        data_ = std::make_shared<Internal::ClientData>(self_name, "127.0.0.1",
                                                       17530);
        wcli_ = std::make_shared<Client>(self_name, data_);
        callback_called = 0;
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
    std::shared_ptr<Internal::ClientData> data_;
    std::shared_ptr<Client> wcli_;
    std::shared_ptr<DummyServer> dummy_s;
    int callback_called = 0;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(ClientTest, unixSocketConnection) {
    auto dummy_tcp_s = std::make_shared<DummyServer>(false);
    dummy_s = std::make_shared<DummyServer>(true);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->start();
    wait();
    dummy_tcp_s.reset();
    wait();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());
}
TEST_F(ClientTest, connectionByStart) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->start();
    wait();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());
}
TEST_F(ClientTest, connectionByWait) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->waitConnection();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());
}
TEST_F(ClientTest, noAutoReconnect) {
    EXPECT_TRUE(wcli_->autoReconnect());
    EXPECT_FALSE(wcli_->connected());
    wcli_->autoReconnect(false);
    EXPECT_FALSE(wcli_->autoReconnect());
    wcli_->waitConnection();
    EXPECT_FALSE(wcli_->connected());

    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->waitConnection();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());

    // dummy_s.reset();
    // wait();
    // EXPECT_FALSE(wcli_->connected());

    // dummy_s = std::make_shared<DummyServer>(false);
    // wait();
    // EXPECT_FALSE(dummy_s->connected());
    // EXPECT_FALSE(wcli_->connected());
}
TEST_F(ClientTest, connectionBySync) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->sync();
    wait();
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());
}
TEST_F(ClientTest, close) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_.reset();
    wait();
    EXPECT_FALSE(dummy_s->connected());
}
TEST_F(ClientTest, name) { EXPECT_EQ(wcli_->name(), self_name); }
TEST_F(ClientTest, memoryLeak) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_.reset();
    wait();
    EXPECT_EQ(data_.use_count(), 1);
}
TEST_F(ClientTest, sync) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->sync();
    wait();
    using namespace webcface::Message;
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
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    dummy_s->send(Message::SvrVersion{{}, "a", "1"});
    wait();
    EXPECT_EQ(wcli_->serverName(), "a");
    EXPECT_EQ(wcli_->serverVersion(), "1");
}
TEST_F(ClientTest, ping) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
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
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
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
    ASSERT_EQ(m.valueEntries().size(), 1);
    EXPECT_EQ(m.valueEntries()[0].name(), "b");

    m.onTextEntry().appendListener(callback<Text>());
    dummy_s->send(Message::Entry<Message::Text>{{}, 10, "c"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.textEntries().size(), 1);
    EXPECT_EQ(m.textEntries()[0].name(), "c");

    m.onViewEntry().appendListener(callback<View>());
    dummy_s->send(Message::Entry<Message::View>{{}, 10, "d"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.viewEntries().size(), 1);
    EXPECT_EQ(m.viewEntries()[0].name(), "d");

    m.onCanvas2DEntry().appendListener(callback<Canvas2D>());
    dummy_s->send(Message::Entry<Message::Canvas2D>{{}, 10, "d"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.canvas2DEntries().size(), 1);
    EXPECT_EQ(m.canvas2DEntries()[0].name(), "d");

    m.onCanvas3DEntry().appendListener(callback<Canvas3D>());
    dummy_s->send(Message::Entry<Message::Canvas3D>{{}, 10, "d"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.canvas3DEntries().size(), 1);
    EXPECT_EQ(m.canvas3DEntries()[0].name(), "d");

    m.onRobotModelEntry().appendListener(callback<RobotModel>());
    dummy_s->send(Message::Entry<Message::RobotModel>{{}, 10, "d"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.robotModelEntries().size(), 1);
    EXPECT_EQ(m.robotModelEntries()[0].name(), "d");

    m.onImageEntry().appendListener(callback<Image>());
    dummy_s->send(Message::Entry<Message::Image>{{}, 10, "d"});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.imageEntries().size(), 1);
    EXPECT_EQ(m.imageEntries()[0].name(), "d");

    m.onFuncEntry().appendListener(callback<Func>());
    dummy_s->send(Message::FuncInfo{
        10, "a", ValType::int_,
        std::make_shared<std::vector<Message::FuncInfo::Arg>>(1)});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(m.funcEntries().size(), 1);
    EXPECT_EQ(m.funcEntries()[0].name(), "a");

    m.onSync().appendListener(callback<Member>());
    dummy_s->send(Message::Sync{10, std::chrono::system_clock::now()});
    wait();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ClientTest, valueSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
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
    dummy_s->recvClear();

    data_->value_store.setSend("a", std::make_shared<VectorOpt<double>>(5));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Value>(
        [&](const auto &) { ADD_FAILURE() << "should not receive same Value"; },
        [&] {});
    dummy_s->recvClear();

    data_->value_store.setSend(
        "a", std::make_shared<VectorOpt<double>>(std::array<double, 2>{5, 2}));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Value>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            ASSERT_EQ(obj.data->size(), 2);
            EXPECT_EQ(obj.data->at(0), 5);
            EXPECT_EQ(obj.data->at(1), 2);
        },
        [&] { ADD_FAILURE() << "Value recv error"; });
}
TEST_F(ClientTest, valueReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").value("b").tryGet();
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
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    data_->text_store.setSend("a", std::make_shared<ValAdaptor>("b"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(*obj.data, "b");
        },
        [&] { ADD_FAILURE() << "Text recv error"; });
    dummy_s->recvClear();

    data_->text_store.setSend("a", std::make_shared<ValAdaptor>("b"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &) { ADD_FAILURE() << "should not receive same Text"; },
        [&] {});
    dummy_s->recvClear();

    data_->text_store.setSend("a", std::make_shared<ValAdaptor>("c"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(*obj.data, "c");
        },
        [&] { ADD_FAILURE() << "Text recv error"; });
}
TEST_F(ClientTest, textReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").text("b").tryGet();
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
        Message::Res<Message::Text>{1, "", std::make_shared<ValAdaptor>("z")});
    dummy_s->send(
        Message::Res<Message::Text>{1, "c", std::make_shared<ValAdaptor>("z")});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->text_store.getRecv("a", "b").has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a", "b").value(), "z");
    EXPECT_TRUE(data_->text_store.getRecv("a", "b.c").has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a", "b.c").value(), "z");
}
TEST_F(ClientTest, viewSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    data_->view_store.setSend(
        "a", std::make_shared<std::vector<ViewComponentBase>>(
                 std::vector<ViewComponentBase>{
                     ViewComponents::text("a")
                         .textColor(ViewColor::yellow)
                         .bgColor(ViewColor::green)
                         .toV()
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
                         .toV()
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
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").view("b").tryGet();
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
                      .toV()
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
                      .toV()
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
TEST_F(ClientTest, canvas2DSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    data_->canvas2d_store.setSend(
        "a", std::make_shared<Canvas2DDataBase>(
                 100, 100,
                 std::vector<Canvas2DComponentBase>{
                     Geometries::line({0, 0}, {30, 30})
                         .color(ViewColor::black)
                         .fillColor(ViewColor::white)
                         .strokeWidth(5)
                         .onClick(Func{Field{data_, self_name, "f"}})
                         .to2()
                         .lockTmp(data_, "", nullptr),
                     Geometries::rect({0, 0}, {30, 30})
                         .color(ViewColor::black)
                         .fillColor(ViewColor::white)
                         .strokeWidth(5)
                         .onClick(Func{Field{data_, self_name, "f"}})
                         .to2()
                         .lockTmp(data_, "", nullptr),
                     Geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                         .color(ViewColor::black)
                         .fillColor(ViewColor::white)
                         .strokeWidth(5)
                         .onClick(Func{Field{data_, self_name, "f"}})
                         .to2()
                         .lockTmp(data_, "", nullptr),
                 }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Canvas2D>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            ASSERT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ(obj.width, 100);
            EXPECT_EQ(obj.height, 100);
            EXPECT_EQ((*obj.data_diff)["0"].type,
                      Canvas2DComponentType::geometry);
            EXPECT_EQ((*obj.data_diff)["0"].color, ViewColor::black);
            EXPECT_EQ((*obj.data_diff)["0"].fill, ViewColor::white);
            EXPECT_EQ((*obj.data_diff)["0"].properties,
                      (std::vector<double>{0, 0, 0, 30, 30, 0}));
            EXPECT_EQ((*obj.data_diff)["0"].geometry_type, GeometryType::line);
            EXPECT_EQ((*obj.data_diff)["0"].on_click_member, self_name);
            EXPECT_EQ((*obj.data_diff)["0"].on_click_field, "f");
            EXPECT_EQ((*obj.data_diff)["1"].geometry_type, GeometryType::rect);
            EXPECT_EQ((*obj.data_diff)["2"].geometry_type,
                      GeometryType::polygon);
        },
        [&] { ADD_FAILURE() << "Canvas2D recv error"; });
    dummy_s->recvClear();

    data_->canvas2d_store.setSend(
        "a", std::make_shared<Canvas2DDataBase>(
                 100, 100,
                 std::vector<Canvas2DComponentBase>{
                     Geometries::line({0, 0}, {30, 30})
                         .color(ViewColor::red) // changed
                         .fillColor(ViewColor::white)
                         .strokeWidth(5)
                         .onClick(Func{Field{data_, self_name, "f"}})
                         .to2()
                         .lockTmp(data_, "", nullptr),
                     Geometries::rect({0, 0}, {30, 30})
                         .color(ViewColor::black)
                         .fillColor(ViewColor::white)
                         .strokeWidth(5)
                         .onClick(Func{Field{data_, self_name, "f"}})
                         .to2()
                         .lockTmp(data_, "", nullptr),
                     Geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                         .color(ViewColor::black)
                         .fillColor(ViewColor::white)
                         .strokeWidth(5)
                         .onClick(Func{Field{data_, self_name, "f"}})
                         .to2()
                         .lockTmp(data_, "", nullptr),
                 }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Canvas2D>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            ASSERT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ((*obj.data_diff)["0"].type,
                      Canvas2DComponentType::geometry);
            EXPECT_EQ((*obj.data_diff)["0"].color, ViewColor::red);
            EXPECT_EQ((*obj.data_diff)["0"].fill, ViewColor::white);
            EXPECT_EQ((*obj.data_diff)["0"].properties,
                      (std::vector<double>{0, 0, 0, 30, 30, 0}));
        },
        [&] { ADD_FAILURE() << "Canvas2D recv error"; });
}
TEST_F(ClientTest, canvas2DReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").canvas2D("b").tryGet();
    wait();
    wcli_->member("a").canvas2D("b").appendListener(callback<Canvas2D>());
    dummy_s->recv<Message::Req<Message::Canvas2D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Canvas2D Req recv error"; });

    auto v = std::make_shared<
        std::unordered_map<std::string, Message::Canvas2D::Canvas2DComponent>>(
        std::unordered_map<std::string, Message::Canvas2D::Canvas2DComponent>{
            {"0", Geometries::line({0, 0}, {30, 30})
                      .color(ViewColor::black)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"}})
                      .to2()
                      .lockTmp(data_, "", nullptr)},
            {"1", Geometries::rect({0, 0}, {30, 30})
                      .color(ViewColor::black)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"}})
                      .to2()
                      .lockTmp(data_, "", nullptr)},
            {"2", Geometries::polygon({{0, 0}, {30, 30}, {50, 20}})
                      .color(ViewColor::black)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"}})
                      .to2()
                      .lockTmp(data_, "", nullptr)},
        });
    dummy_s->send(Message::Res<Message::Canvas2D>{1, "", 200, 200, v, 3});
    dummy_s->send(Message::Res<Message::Canvas2D>{1, "c", 200, 200, v, 3});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas2d_store.getRecv("a", "b").has_value());
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b").value()->width, 200);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b").value()->height, 200);
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.size(), 3);
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.at(0).type_,
        Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .color_,
              ViewColor::black);
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.at(0).fill_,
        ViewColor::white);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .geometry_->type,
              GeometryType::line);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .geometry_->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 0}));
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .on_click_func_->member_,
              self_name);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .on_click_func_->field_,
              "f");
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.at(1).type_,
        Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(1)
                  .geometry_->type,
              GeometryType::rect);
    EXPECT_TRUE(data_->canvas2d_store.getRecv("a", "b.c").has_value());

    // 差分だけ送る
    auto v2 = std::make_shared<
        std::unordered_map<std::string, Message::Canvas2D::Canvas2DComponent>>(
        std::unordered_map<std::string, Message::Canvas2D::Canvas2DComponent>{
            {"0", Geometries::line({0, 0}, {30, 30})
                      .color(ViewColor::red)
                      .fillColor(ViewColor::white)
                      .strokeWidth(5)
                      .onClick(Func{Field{data_, self_name, "f"}})
                      .to2()
                      .lockTmp(data_, "", nullptr)},
        });
    dummy_s->send(Message::Res<Message::Canvas2D>{1, "", 100, 100, v2, 3});
    wait();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.size(), 3);
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.at(0).type_,
        Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .color_,
              ViewColor::red);
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.at(0).fill_,
        ViewColor::white);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .geometry_->type,
              GeometryType::line);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(0)
                  .geometry_->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 0}));
    EXPECT_EQ(
        data_->canvas2d_store.getRecv("a", "b").value()->components.at(1).type_,
        Canvas2DComponentType::geometry);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a", "b")
                  .value()
                  ->components.at(1)
                  .geometry_->type,
              GeometryType::rect);
}
TEST_F(ClientTest, canvas3DSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    data_->canvas3d_store.setSend(
        "a",
        std::make_shared<std::vector<Canvas3DComponentBase>>(
            std::vector<Canvas3DComponentBase>{
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(
                     Geometries::line({0, 0, 0}, {30, 30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(
                     Geometries::rect({0, 0}, {30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(Geometries::sphere({0, 0, 0}, 1)),
                 std::nullopt,
                 {}},
            }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Canvas3D>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            ASSERT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ((*obj.data_diff)["0"].type,
                      Canvas3DComponentType::geometry);
            EXPECT_EQ((*obj.data_diff)["0"].color, ViewColor::black);
            EXPECT_EQ((*obj.data_diff)["0"].geometry_properties,
                      (std::vector<double>{0, 0, 0, 30, 30, 30}));
            EXPECT_EQ((*obj.data_diff)["0"].geometry_type, GeometryType::line);
            EXPECT_EQ((*obj.data_diff)["1"].geometry_type, GeometryType::rect);
            EXPECT_EQ((*obj.data_diff)["2"].geometry_type,
                      GeometryType::sphere);
        },
        [&] { ADD_FAILURE() << "Canvas3D recv error"; });
    dummy_s->recvClear();

    data_->canvas3d_store.setSend(
        "a",
        std::make_shared<std::vector<Canvas3DComponentBase>>(
            std::vector<Canvas3DComponentBase>{
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::red,
                 std::make_optional<Geometry>(
                     Geometries::line({0, 0, 0}, {30, 30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(
                     Geometries::rect({0, 0}, {30, 30})),
                 std::nullopt,
                 {}},
                {Canvas3DComponentType::geometry,
                 identity(),
                 ViewColor::black,
                 std::make_optional<Geometry>(Geometries::sphere({0, 0, 0}, 1)),
                 std::nullopt,
                 {}},
            }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Canvas3D>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            ASSERT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ((*obj.data_diff)["0"].type,
                      Canvas3DComponentType::geometry);
            EXPECT_EQ((*obj.data_diff)["0"].color, ViewColor::red);
            EXPECT_EQ((*obj.data_diff)["0"].geometry_properties,
                      (std::vector<double>{0, 0, 0, 30, 30, 30}));
            EXPECT_EQ((*obj.data_diff)["0"].geometry_type, GeometryType::line);
        },
        [&] { ADD_FAILURE() << "Canvas3D recv error"; });
}
TEST_F(ClientTest, canvas3DReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").canvas3D("b").tryGet();
    wait();
    wcli_->member("a").canvas3D("b").appendListener(callback<Canvas3D>());
    dummy_s->recv<Message::Req<Message::Canvas3D>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Canvas3D Req recv error"; });

    auto v = std::make_shared<
        std::unordered_map<std::string, Message::Canvas3D::Canvas3DComponent>>(
        std::unordered_map<std::string, Message::Canvas3D::Canvas3DComponent>{
            {"0", Canvas3DComponentBase{Canvas3DComponentType::geometry,
                                        identity(),
                                        ViewColor::black,
                                        std::make_optional<Geometry>(
                                            Geometries::line({0, 0, 0},
                                                             {30, 30, 30})),
                                        std::nullopt,
                                        {}}},
            {"1", Canvas3DComponentBase{Canvas3DComponentType::geometry,
                                        identity(),
                                        ViewColor::black,
                                        std::make_optional<Geometry>(
                                            Geometries::rect({0, 0}, {30, 30})),
                                        std::nullopt,
                                        {}}},
            {"2", Canvas3DComponentBase{Canvas3DComponentType::geometry,
                                        identity(),
                                        ViewColor::black,
                                        std::make_optional<Geometry>(
                                            Geometries::sphere({0, 0, 0}, 1)),
                                        std::nullopt,
                                        {}}},
        });
    dummy_s->send(Message::Res<Message::Canvas3D>{1, "", v, 3});
    dummy_s->send(Message::Res<Message::Canvas3D>{1, "c", v, 3});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a", "b").has_value());
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->size(), 3);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->at(0).type_,
              Canvas3DComponentType::geometry);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->at(0).color_,
              ViewColor::black);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a", "b").value()->at(0).geometry_->type,
        GeometryType::line);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b")
                  .value()
                  ->at(0)
                  .geometry_->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 30}));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->at(1).type_,
              Canvas3DComponentType::geometry);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a", "b").value()->at(1).geometry_->type,
        GeometryType::rect);
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a", "b.c").has_value());

    // 差分だけ送る
    auto v2 = std::make_shared<
        std::unordered_map<std::string, Message::Canvas3D::Canvas3DComponent>>(
        std::unordered_map<std::string, Message::Canvas3D::Canvas3DComponent>{
            {"0", Canvas3DComponentBase{Canvas3DComponentType::geometry,
                                        identity(),
                                        ViewColor::red,
                                        std::make_optional<Geometry>(
                                            Geometries::line({0, 0, 0},
                                                             {30, 30, 30})),
                                        std::nullopt,
                                        {}}},
        });
    dummy_s->send(Message::Res<Message::Canvas3D>{1, "", v2, 3});
    wait();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->size(), 3);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->at(0).type_,
              Canvas3DComponentType::geometry);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->at(0).color_,
              ViewColor::red);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a", "b").value()->at(0).geometry_->type,
        GeometryType::line);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b")
                  .value()
                  ->at(0)
                  .geometry_->properties,
              (std::vector<double>{0, 0, 0, 30, 30, 30}));
    EXPECT_EQ(data_->canvas3d_store.getRecv("a", "b").value()->at(1).type_,
              Canvas3DComponentType::geometry);
    EXPECT_EQ(
        data_->canvas3d_store.getRecv("a", "b").value()->at(1).geometry_->type,
        GeometryType::rect);
}
TEST_F(ClientTest, robotModelSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    data_->robot_model_store.setSend(
        "a", std::make_shared<std::vector<RobotLink>>(
                 std::vector<RobotLink>{{"a", Geometry{}, ViewColor::black}}));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::RobotModel>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.data->size(), 1);
        },
        [&] { ADD_FAILURE() << "RobotModel recv error"; });
}
TEST_F(ClientTest, robotModelReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").robotModel("b").tryGet();
    wait();
    wcli_->member("a").robotModel("b").appendListener(callback<RobotModel>());
    dummy_s->recv<Message::Req<Message::RobotModel>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "RobotModel Req recv error"; });
    dummy_s->send(Message::Res<Message::RobotModel>(
        1, "",
        std::make_shared<std::vector<RobotLink>>(
            std::vector<RobotLink>{{"a", Geometry{}, ViewColor::black}})));
    dummy_s->send(Message::Res<Message::RobotModel>(
        1, "c",
        std::make_shared<std::vector<RobotLink>>(
            std::vector<RobotLink>{{"a", Geometry{}, ViewColor::black}})));
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->robot_model_store.getRecv("a", "b").has_value());
    EXPECT_EQ(data_->robot_model_store.getRecv("a", "b").value()->size(), 1);
    EXPECT_TRUE(data_->robot_model_store.getRecv("a", "b.c").has_value());
    EXPECT_EQ(data_->robot_model_store.getRecv("a", "b.c").value()->size(), 1);
}
TEST_F(ClientTest, imageSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    data_->image_store.setSend(
        "a", ImageFrame{
                 100, 100,
                 std::make_shared<std::vector<unsigned char>>(100 * 100 * 3)});
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Image>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.data().size(), 100 * 100 * 3);
        },
        [&] { ADD_FAILURE() << "Image recv error"; });
}
TEST_F(ClientTest, imageReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").image("b").tryGet();
    wait();
    wcli_->member("a").image("b").appendListener(callback<Image>());
    dummy_s->recv<Message::Req<Message::Image>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
            EXPECT_EQ(static_cast<ImageReq>(obj),
                      (ImageReq{std::nullopt, std::nullopt, std::nullopt,
                                ImageCompressMode::raw, 0, std::nullopt}));
        },
        [&] { ADD_FAILURE() << "Image Req recv error"; });
    ImageFrame img(100, 100,
                   std::make_shared<std::vector<unsigned char>>(100 * 100 * 3));
    dummy_s->send(Message::Res<Message::Image>{1, "", img});
    dummy_s->send(Message::Res<Message::Image>{1, "c", img});
    wait();
    EXPECT_EQ(callback_called, 1);
    ASSERT_TRUE(data_->image_store.getRecv("a", "b").has_value());
    EXPECT_EQ(data_->image_store.getRecv("a", "b")->data().size(),
              img.data().size());
    ASSERT_TRUE(data_->image_store.getRecv("a", "b.c").has_value());
    EXPECT_EQ(data_->image_store.getRecv("a", "b.c")->data().size(),
              img.data().size());
}
TEST_F(ClientTest, logSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    auto ls = std::make_shared<std::vector<Common::LogLine>>(
        std::vector<Common::LogLine>{
            {0, std::chrono::system_clock::now(), std::string(100000, 'a')},
            {1, std::chrono::system_clock::now(), "b"},
        });
    data_->log_store->setRecv(self_name, ls);
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Log>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.log->size(), 2);
            EXPECT_EQ(obj.log->at(0).level, 0);
            EXPECT_EQ(obj.log->at(0).message.size(), 100000);
            EXPECT_EQ(obj.log->at(1).level, 1);
            EXPECT_EQ(obj.log->at(1).message, "b");
        },
        [&] { ADD_FAILURE() << "Log recv error"; });

    dummy_s->recvClear();
    ls->push_back(LogLine{2, std::chrono::system_clock::now(), "c"});
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
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
    wcli_->member("a").log().tryGet();
    wait();
    wcli_->member("a").log().appendListener(callback<Log>());
    dummy_s->recv<Message::LogReq>(
        [&](const auto &obj) { EXPECT_EQ(obj.member, "a"); },
        [&] { ADD_FAILURE() << "Log Req recv error"; });

    dummy_s->send(Message::SyncInit{{}, "a", 10, "", "", ""});
    dummy_s->send(Message::Log{
        10, std::make_shared<std::deque<Message::Log::LogLine>>(
                std::deque<Message::Log::LogLine>{
                    LogLine{0, std::chrono::system_clock::now(),
                            std::string(100000, 'a')},
                    LogLine{1, std::chrono::system_clock::now(), "b"},
                })});
    wait();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->log_store->getRecv("a").has_value());
    EXPECT_EQ(data_->log_store->getRecv("a").value()->size(), 2);
    EXPECT_EQ(data_->log_store->getRecv("a").value()->at(0).level, 0);
    EXPECT_EQ(data_->log_store->getRecv("a").value()->at(0).message.size(),
              100000);

    dummy_s->send(Message::Log{
        10, std::make_shared<std::deque<Message::Log::LogLine>>(
                std::deque<Message::Log::LogLine>{
                    LogLine{2, std::chrono::system_clock::now(), "c"},
                })});
    wait();
    EXPECT_EQ(callback_called, 2);
    EXPECT_TRUE(data_->log_store->getRecv("a").has_value());
    EXPECT_EQ(data_->log_store->getRecv("a").value()->size(), 3);
}
TEST_F(ClientTest, funcInfo) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
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
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
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
    dummy_s->send(Message::CallResult{{}, 1, 0, true, ValAdaptor("a")});
    wait();
    EXPECT_THROW(r.result.get(), std::runtime_error);
    try {
        r.result.get();
    } catch (const std::runtime_error &e) {
        using namespace std::string_literals;
        EXPECT_EQ(e.what(), "a"s);
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
    dummy_s->send(Message::CallResult{{}, 2, 0, false, ValAdaptor("b")});
    wait();
    EXPECT_EQ(static_cast<std::string>(r.result.get()), "b");
}
TEST_F(ClientTest, funcResponse) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->waitConnection();
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
    dummy_s->send(Message::Call{
        FuncCall{8, 100, 0, "a", {ValAdaptor(1), ValAdaptor("zzz")}}});
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
    dummy_s->send(Message::Call{FuncCall{9, 100, 0, "a", {ValAdaptor(0)}}});
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
    dummy_s->send(Message::Call{FuncCall{19, 100, 0, "a", {ValAdaptor(123)}}});
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
