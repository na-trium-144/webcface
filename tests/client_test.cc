#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/client.h>
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
#include "webcface/message/message.h"
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
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class ClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        data_ = std::make_shared<internal::ClientData>(self_name,
                                                       "127.0.0.1"_ss, 17530);
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
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
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
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
}
TEST_F(ClientTest, connectionByStart) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
}
TEST_F(ClientTest, connectionByWait) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    std::promise<void> p;
    auto f = p.get_future();
    std::thread t([&] {
        wcli_->waitConnection();
        p.set_value();
    });
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    dummy_s->waitRecv<message::SyncInit>([&](auto) {});
    EXPECT_NE(f.wait_for(std::chrono::milliseconds(0)),
              std::future_status::ready);
    dummy_s->send(message::SyncInitEnd{{}, "", "", 0, ""});
    t.join();
    f.get();
    EXPECT_TRUE(wcli_->connected());
}
TEST_F(ClientTest, noAutoReconnect) {
    EXPECT_TRUE(wcli_->autoReconnect());
    EXPECT_FALSE(wcli_->connected());
    wcli_->autoReconnect(false);
    EXPECT_FALSE(wcli_->autoReconnect());
    wcli_->waitConnection();
    EXPECT_FALSE(wcli_->connected());

    dummy_s.reset();
    while (wcli_->connected()) {
        wait();
    }

    dummy_s = std::make_shared<DummyServer>(false);
    std::thread t([&] { wcli_->waitConnection(); });
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    dummy_s->waitRecv<message::SyncInit>([&](auto) {});
    dummy_s->send(message::SyncInitEnd{{}, "", "", 0, ""});
    t.join();
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
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
}
TEST_F(ClientTest, noConnectionByRecv) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->recv();
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
}
TEST_F(ClientTest, close) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_.reset();
    wait();
    EXPECT_FALSE(dummy_s->connected());
}
TEST_F(ClientTest, name) {
    EXPECT_EQ(wcli_->name(), self_name.decode());
    EXPECT_EQ(wcli_->nameW(), self_name.decodeW());
}
TEST_F(ClientTest, memoryLeak) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_.reset();
    wait();
    EXPECT_EQ(data_.use_count(), 1);
}
TEST_F(ClientTest, sync) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->sync();
    using namespace webcface::message;
    dummy_s->waitRecv<SyncInit>([&](const auto &obj) {
        EXPECT_EQ(obj.member_name, self_name);
        EXPECT_EQ(obj.lib_name, "cpp");
        EXPECT_EQ(obj.lib_ver, WEBCFACE_VERSION);
    });
    dummy_s->waitRecv<Sync>([&](const auto &) {});
    // todo: 時刻が正しく変換できてるかテストする(めんどくさい)

    dummy_s->recvClear();
    wcli_->sync();
    dummy_s->waitRecv<Sync>([&](const auto &) {});
    dummy_s->recv<SyncInit>(
        [&](const auto &) {
            ADD_FAILURE() << "should not send SyncInit twice";
        },
        [&] {});
}
TEST_F(ClientTest, serverVersion) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    dummy_s->waitRecv<message::SyncInit>([&](auto) {});
    dummy_s->send(message::SyncInitEnd{{}, "a", "1", 0, "b"});
    wait();
    EXPECT_EQ(wcli_->serverName(), "");
    wcli_->waitRecv();
    EXPECT_EQ(wcli_->serverName(), "a");
    EXPECT_EQ(wcli_->serverVersion(), "1");
    EXPECT_EQ(wcli_->serverHostName(), "b");
}
TEST_F(ClientTest, ping) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    dummy_s->waitRecv<message::SyncInit>([&](auto) {});
    dummy_s->send(message::SyncInitEnd{{}, "", "", 42, ""});
    wcli_->waitRecv();
    dummy_s->send(message::Ping{});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::Ping>([&](const auto &) {});

    wcli_->member("a").onPing(callback<Member>());
    wcli_->onPing(callback<Member>());
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "", "", ""});
    wcli_->waitRecv();
    dummy_s->send(message::PingStatus{
        {},
        std::make_shared<std::unordered_map<unsigned int, int>>(
            std::unordered_map<unsigned int, int>{{10, 15}, {42, 99}})});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::PingStatusReq>([&](const auto &) {});
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(wcli_->member("a").pingStatus().value(), 15);
    EXPECT_EQ(wcli_->pingStatus().value(), 99);
}
TEST_F(ClientTest, entry) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->onMemberEntry(callback<Member>());
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "b", "1", "12345"});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(wcli_->members().size(), 1);
    EXPECT_EQ(wcli_->members()[0].name(), "a");
    EXPECT_EQ(wcli_->members()[0].nameW(), L"a");
    EXPECT_EQ(data_->member_ids["a"_ss], 10);

    auto m = wcli_->member("a");
    EXPECT_EQ(m.libName(), "b");
    EXPECT_EQ(m.libVersion(), "1");
    EXPECT_EQ(m.remoteAddr(), "12345");

    m.onValueEntry(callback<Value>());
    dummy_s->send(message::Entry<message::Value>{{}, 10, "b"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.valueEntries().size(), 1);
    EXPECT_EQ(m.valueEntries()[0].name(), "b");
    EXPECT_EQ(m.valueEntries()[0].nameW(), L"b");

    m.onTextEntry(callback<Text>());
    dummy_s->send(message::Entry<message::Text>{{}, 10, "c"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.textEntries().size(), 1);
    EXPECT_EQ(m.textEntries()[0].name(), "c");
    EXPECT_EQ(m.textEntries()[0].nameW(), L"c");

    m.onViewEntry(callback<View>());
    dummy_s->send(message::Entry<message::View>{{}, 10, "d"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.viewEntries().size(), 1);
    EXPECT_EQ(m.viewEntries()[0].name(), "d");
    EXPECT_EQ(m.viewEntries()[0].nameW(), L"d");

    m.onCanvas2DEntry(callback<Canvas2D>());
    dummy_s->send(message::Entry<message::Canvas2D>{{}, 10, "d"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.canvas2DEntries().size(), 1);
    EXPECT_EQ(m.canvas2DEntries()[0].name(), "d");
    EXPECT_EQ(m.canvas2DEntries()[0].nameW(), L"d");

    m.onCanvas3DEntry(callback<Canvas3D>());
    dummy_s->send(message::Entry<message::Canvas3D>{{}, 10, "d"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.canvas3DEntries().size(), 1);
    EXPECT_EQ(m.canvas3DEntries()[0].name(), "d");
    EXPECT_EQ(m.canvas3DEntries()[0].nameW(), L"d");

    m.onRobotModelEntry(callback<RobotModel>());
    dummy_s->send(message::Entry<message::RobotModel>{{}, 10, "d"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.robotModelEntries().size(), 1);
    EXPECT_EQ(m.robotModelEntries()[0].name(), "d");
    EXPECT_EQ(m.robotModelEntries()[0].nameW(), L"d");

    m.onImageEntry(callback<Image>());
    dummy_s->send(message::Entry<message::Image>{{}, 10, "d"_ss});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    ASSERT_EQ(m.imageEntries().size(), 1);
    EXPECT_EQ(m.imageEntries()[0].name(), "d");
    EXPECT_EQ(m.imageEntries()[0].nameW(), L"d");

    m.onFuncEntry(callback<Func>());
    dummy_s->send(
        message::FuncInfo{10, "a"_ss, ValType::int_,
                          std::make_shared<std::vector<message::Arg>>(1)});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_EQ(m.funcEntries().size(), 1);
    EXPECT_EQ(m.funcEntries()[0].name(), "a");
    EXPECT_EQ(m.funcEntries()[0].nameW(), L"a");

    m.onSync(callback<Member>());
    dummy_s->send(message::Sync{10, std::chrono::system_clock::now()});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ClientTest, valueSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->value_store.setSend(
        "a"_ss, std::make_shared<std::vector<double>>(std::vector<double>{5}));
    wcli_->sync();
    dummy_s->waitRecv<message::Value>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.data->size(), 1);
        EXPECT_EQ(obj.data->at(0), 5);
    });
    dummy_s->recvClear();

    data_->value_store.setSend(
        "a"_ss, std::make_shared<std::vector<double>>(std::vector<double>{5}));
    wcli_->sync();
    wait();
    dummy_s->recv<message::Value>(
        [&](const auto &) { ADD_FAILURE() << "should not receive same Value"; },
        [&] {});
    dummy_s->recvClear();

    data_->value_store.setSend("a"_ss, std::make_shared<std::vector<double>>(
                                           std::vector<double>{5, 2}));
    wcli_->sync();
    dummy_s->waitRecv<message::Value>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        ASSERT_EQ(obj.data->size(), 2);
        EXPECT_EQ(obj.data->at(0), 5);
        EXPECT_EQ(obj.data->at(1), 2);
    });
}
TEST_F(ClientTest, valueReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").value("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Value>>([&](const auto &obj) {
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
    });
    wcli_->member("a").value("b").onChange(callback<Value>());
    dummy_s->send(message::Res<message::Value>{
        1, ""_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::Value>{
        1, "c"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b"_ss).value())
                  .size(),
              3);
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b.c"_ss).value())
                  .size(),
              3);
}
TEST_F(ClientTest, recvThread) {
    dummy_s = std::make_shared<DummyServer>(false);
    auto main_id = std::this_thread::get_id();
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").value("b").onChange([&](const Value &) {
        EXPECT_EQ(std::this_thread::get_id(), main_id);
        callback_called++;
    });
    wait();
    dummy_s->send(message::Res<message::Value>{
        1, ""_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
}
TEST_F(ClientTest, autoRecvThread) {
    dummy_s = std::make_shared<DummyServer>(false);
    auto main_id = std::this_thread::get_id();
    wait();
    wcli_->autoRecv(true);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").value("b").onChange([&](const Value &) {
        EXPECT_NE(std::this_thread::get_id(), main_id);
        callback_called++;
    });
    wait();
    dummy_s->send(message::Res<message::Value>{
        1, ""_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wait();
    EXPECT_EQ(callback_called, 1);
}
TEST_F(ClientTest, recvTimeout) {
    auto start = std::chrono::steady_clock::now();
    wcli_->waitRecvFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    auto end = std::chrono::steady_clock::now();
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.9);
    start = std::chrono::steady_clock::now();
    wcli_->recv();
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
    start = std::chrono::steady_clock::now();
    wcli_->waitRecvFor(std::chrono::milliseconds(-WEBCFACE_TEST_TIMEOUT));
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
}
TEST_F(ClientTest, recvUntilTimeout) {
    auto start = std::chrono::steady_clock::now();
    wcli_->waitRecvUntil(start + std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    auto end = std::chrono::steady_clock::now();
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.9);
    start = std::chrono::steady_clock::now();
    wcli_->waitRecvUntil(start);
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
    start = std::chrono::steady_clock::now();
    wcli_->waitRecvUntil(start - std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
}
TEST_F(ClientTest, textSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->text_store.setSend("a"_ss, std::make_shared<ValAdaptor>("b"));
    wcli_->sync();
    dummy_s->waitRecv<message::Text>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(*obj.data, "b");
    });
    dummy_s->recvClear();

    data_->text_store.setSend("a"_ss, std::make_shared<ValAdaptor>("b"));
    wcli_->sync();
    wait();
    dummy_s->recv<message::Text>(
        [&](const auto &) { ADD_FAILURE() << "should not receive same Text"; },
        [&] {});
    dummy_s->recvClear();

    data_->text_store.setSend("a"_ss, std::make_shared<ValAdaptor>("c"));
    wcli_->sync();
    dummy_s->waitRecv<message::Text>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(*obj.data, "c");
    });
}
TEST_F(ClientTest, textReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").text("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
    });
    wcli_->member("a").text("b").onChange(callback<Text>());
    dummy_s->send(message::Res<message::Text>{
        1, ""_ss, std::make_shared<ValAdaptor>("z")});
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::Text>{
        1, "c"_ss, std::make_shared<ValAdaptor>("z")});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->text_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a"_ss, "b"_ss).value(), "z");
    EXPECT_TRUE(data_->text_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a"_ss, "b.c"_ss).value(), "z");
}
TEST_F(ClientTest, viewSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->view_store.setSend(
        "a"_ss,
        std::make_shared<std::vector<ViewComponent>>(std::vector<ViewComponent>{
            ViewComponents::text("a")
                .textColor(ViewColor::yellow)
                .bgColor(ViewColor::green)
                .toV()
                .lockTmp(data_, ""_ss),
            ViewComponents::newLine().lockTmp(data_, ""_ss),
            ViewComponents::button("a", Func{Field{data_, "x"_ss, "y"_ss}})
                .lockTmp(data_, ""_ss),
        }));
    wcli_->sync();
    dummy_s->waitRecv<message::View>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.length, 3);
        EXPECT_EQ(obj.data_diff->size(), 3);
        EXPECT_EQ((*obj.data_diff)["0"].type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ((*obj.data_diff)["0"].text, "a"_ss);
        EXPECT_EQ((*obj.data_diff)["0"].text_color,
                  static_cast<int>(ViewColor::yellow));
        EXPECT_EQ((*obj.data_diff)["0"].bg_color,
                  static_cast<int>(ViewColor::green));
        EXPECT_EQ((*obj.data_diff)["1"].type,
                  static_cast<int>(ViewComponentType::new_line));
        EXPECT_EQ((*obj.data_diff)["2"].type,
                  static_cast<int>(ViewComponentType::button));
        EXPECT_EQ((*obj.data_diff)["2"].text, "a"_ss);
        EXPECT_EQ((*obj.data_diff)["2"].on_click_member, "x"_ss);
        EXPECT_EQ((*obj.data_diff)["2"].on_click_field, "y"_ss);
    });
    dummy_s->recvClear();

    data_->view_store.setSend(
        "a"_ss,
        std::make_shared<std::vector<ViewComponent>>(std::vector<ViewComponent>{
            ViewComponents::text("b")
                .textColor(ViewColor::red)
                .bgColor(ViewColor::green)
                .toV()
                .lockTmp(data_, ""_ss),
            ViewComponents::newLine().lockTmp(data_, ""_ss),
            ViewComponents::button("a", Func{Field{data_, "x"_ss, "y"_ss}})
                .lockTmp(data_, ""_ss),
        }));
    wcli_->sync();
    dummy_s->waitRecv<message::View>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.length, 3);
        EXPECT_EQ(obj.data_diff->size(), 1);
        EXPECT_EQ((*obj.data_diff)["0"].type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ((*obj.data_diff)["0"].text, "b"_ss);
        EXPECT_EQ((*obj.data_diff)["0"].text_color,
                  static_cast<int>(ViewColor::red));
        EXPECT_EQ((*obj.data_diff)["0"].bg_color,
                  static_cast<int>(ViewColor::green));
    });
}
TEST_F(ClientTest, viewReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").view("b").tryGet();
    dummy_s->waitRecv<message::Req<message::View>>([&](const auto &obj) {
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
    });
    wcli_->member("a").view("b").onChange(callback<View>());

    auto v = std::make_shared<
        std::unordered_map<std::string, message::ViewComponent>>(
        std::unordered_map<std::string, message::ViewComponent>{
            {"0", ViewComponents::text("a")
                      .textColor(ViewColor::yellow)
                      .bgColor(ViewColor::green)
                      .toV()
                      .lockTmp(data_, ""_ss)
                      .toMessage()},
            {"1", ViewComponents::newLine().lockTmp(data_, ""_ss).toMessage()},
            {"2",
             ViewComponents::button("a", Func{Field{data_, "x"_ss, "y"_ss}})
                 .lockTmp(data_, ""_ss)
                 .toMessage()},
        });
    dummy_s->send(message::Res<message::View>{1, ""_ss, v, 3});
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::View>{1, "c"_ss, v, 3});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->view_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->size(), 3);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).type(),
              ViewComponentType::text);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).text(),
              "a");
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).textColor(),
        ViewColor::yellow);
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).bgColor(),
        ViewColor::green);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(1).type(),
              ViewComponentType::new_line);
    EXPECT_TRUE(data_->view_store.getRecv("a"_ss, "b.c"_ss).has_value());

    // 差分だけ送る
    auto v2 = std::make_shared<
        std::unordered_map<std::string, message::ViewComponent>>(
        std::unordered_map<std::string, message::ViewComponent>{
            {"0", ViewComponents::text("b")
                      .textColor(ViewColor::red)
                      .bgColor(ViewColor::green)
                      .toV()
                      .lockTmp(data_, ""_ss)
                      .toMessage()},
        });
    dummy_s->send(message::Res<message::View>{1, ""_ss, v2, 3});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->size(), 3);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).type(),
              ViewComponentType::text);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).text(),
              "b");
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).textColor(),
        ViewColor::red);
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(0).bgColor(),
        ViewColor::green);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->at(1).type(),
              ViewComponentType::new_line);
}
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
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.length, 3);
        ASSERT_EQ(obj.data_diff->size(), 3);
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
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.length, 3);
        ASSERT_EQ(obj.data_diff->size(), 1);
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
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
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
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::Canvas2D>{1, "c"_ss, 200, 200, v, 3});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas2d_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss).value()->width,
              200);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss).value()->height,
              200);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3);
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
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas2d_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->components.size(),
              3);
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
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.length, 3);
        ASSERT_EQ(obj.data_diff->size(), 3);
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
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.length, 3);
        ASSERT_EQ(obj.data_diff->size(), 1);
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
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
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
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::Canvas3D>{1, "c"_ss, v, 3});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->size(), 3);
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
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->canvas3d_store.getRecv("a"_ss, "b"_ss).value()->size(), 3);
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
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.data->size(), 1);
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
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
    });
    wcli_->member("a").robotModel("b").onChange(callback<RobotModel>());
    dummy_s->send(message::Res<message::RobotModel>(
        1, ""_ss,
        std::make_shared<std::vector<message::RobotLink>>(
            std::vector<message::RobotLink>{
                RobotLink{"a", Geometry{}, ViewColor::black}.toMessage({})})));
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::RobotModel>(
        1, "c"_ss,
        std::make_shared<std::vector<message::RobotLink>>(
            std::vector<message::RobotLink>{
                RobotLink{"a", Geometry{}, ViewColor::black}.toMessage({})})));
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->robot_model_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->robot_model_store.getRecv("a"_ss, "b"_ss).value()->size(),
              1);
    EXPECT_TRUE(data_->robot_model_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(
        data_->robot_model_store.getRecv("a"_ss, "b.c"_ss).value()->size(), 1);
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
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.data_->size(), 100 * 100 * 3);
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
        EXPECT_EQ(obj.member, "a"_ss);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.req_id, 1);
        EXPECT_EQ(obj,
                  (message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                                     ImageCompressMode::raw, 0, std::nullopt}));
    });
    wcli_->member("a").image("b").onChange(callback<Image>());
    ImageFrame img(sizeWH(100, 100),
                   std::make_shared<std::vector<unsigned char>>(100 * 100 * 3),
                   ImageColorMode::bgr);
    dummy_s->send(message::Res<message::Image>{1, ""_ss, img.toMessage()});
    wcli_->waitRecv();
    dummy_s->send(message::Res<message::Image>{1, "c"_ss, img.toMessage()});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    ASSERT_TRUE(data_->image_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->image_store.getRecv("a"_ss, "b"_ss)->data().size(),
              img.data().size());
    ASSERT_TRUE(data_->image_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(data_->image_store.getRecv("a"_ss, "b.c"_ss)->data().size(),
              img.data().size());
}
TEST_F(ClientTest, logSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto ls =
        std::make_shared<std::vector<LogLineData>>(std::vector<LogLineData>{
            {0, std::chrono::system_clock::now(),
             SharedString::fromU8String(std::string(100000, 'a'))},
            {1, std::chrono::system_clock::now(), "b"_ss},
        });
    data_->log_store.setRecv(self_name, ls);
    wcli_->sync();
    dummy_s->waitRecv<message::Log>([&](const auto &obj) {
        EXPECT_EQ(obj.log->size(), 2);
        EXPECT_EQ(obj.log->at(0).level_, 0);
        EXPECT_EQ(obj.log->at(0).message_.decode().size(), 100000);
        EXPECT_EQ(obj.log->at(1).level_, 1);
        EXPECT_EQ(obj.log->at(1).message_, "b"_ss);
    });

    dummy_s->recvClear();
    ls->push_back(LogLineData{2, std::chrono::system_clock::now(), "c"_ss});
    wcli_->sync();
    dummy_s->waitRecv<message::Log>([&](const auto &obj) {
        EXPECT_EQ(obj.log->size(), 1);
        EXPECT_EQ(obj.log->at(0).level_, 2);
        EXPECT_EQ(obj.log->at(0).message_, "c"_ss);
    });
}
TEST_F(ClientTest, logReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").log().tryGet();
    dummy_s->waitRecv<message::LogReq>(
        [&](const auto &obj) { EXPECT_EQ(obj.member, "a"_ss); });
    wcli_->member("a").log().onChange(callback<Log>());

    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "", "", ""});
    wcli_->waitRecv();
    dummy_s->send(message::Log{
        10, std::make_shared<std::deque<message::LogLine>>(
                std::deque<message::LogLine>{
                    LogLineData{0, std::chrono::system_clock::now(),
                                  SharedString::fromU8String(std::string(100000, 'a'))}
                        .toMessage(),
                    LogLineData{1, std::chrono::system_clock::now(), "b"_ss}
                        .toMessage(),
                })});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->log_store.getRecv("a"_ss).has_value());
    EXPECT_EQ(data_->log_store.getRecv("a"_ss).value()->size(), 2);
    EXPECT_EQ(data_->log_store.getRecv("a"_ss).value()->at(0).level_, 0);
    EXPECT_EQ(data_->log_store.getRecv("a"_ss).value()->at(0).message_.u8String().size(),
              100000);

    dummy_s->send(message::Log{
        10, std::make_shared<std::deque<message::LogLine>>(
                std::deque<message::LogLine>{
                    LogLineData{2, std::chrono::system_clock::now(), "c"_ss}
                        .toMessage(),
                })});
    wcli_->waitRecv();
    EXPECT_EQ(callback_called, 2);
    EXPECT_TRUE(data_->log_store.getRecv("a"_ss).has_value());
    EXPECT_EQ(data_->log_store.getRecv("a"_ss).value()->size(), 3);
}
TEST_F(ClientTest, funcInfo) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto f =
        wcli_->func("a").set([](int) { return 1; }).setArgs({Arg("a").init(3)});
    wcli_->sync();
    dummy_s->waitRecv<message::FuncInfo>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.return_type, ValType::int_);
        EXPECT_EQ(obj.args->size(), 1);
        EXPECT_EQ(obj.args->at(0).name_, "a"_ss);
    });
}
TEST_F(ClientTest, funcCall) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    // call
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "", "", ""});
    wcli_->waitRecv();
    auto r = wcli_->member("a").func("b").runAsync(1, true, "a");
    dummy_s->waitRecv<message::Call>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 0);
        EXPECT_EQ(obj.target_member_id, 10);
        EXPECT_EQ(obj.field, "b"_ss);
        EXPECT_EQ(obj.args.size(), 3);
        EXPECT_EQ(static_cast<int>(obj.args[0]), 1);
        EXPECT_EQ(obj.args[0].valType(), ValType::int_);
        EXPECT_EQ(static_cast<bool>(obj.args[1]), true);
        EXPECT_EQ(obj.args[1].valType(), ValType::bool_);
        EXPECT_EQ(static_cast<std::string>(obj.args[2]), "a");
        EXPECT_EQ(obj.args[2].valType(), ValType::string_);
    });

    // started=false
    dummy_s->send(message::CallResponse{{}, 0, 0, false});
    wcli_->waitRecv();
    EXPECT_FALSE(r.started.get());
    EXPECT_THROW(r.result.get(), FuncNotFound);
    dummy_s->recvClear();

    // 2nd call id=1
    r = wcli_->member("a").func("b").runAsync(1, true, "a");
    dummy_s->waitRecv<message::Call>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 1);
        EXPECT_EQ(obj.target_member_id, 10);
        EXPECT_EQ(obj.field, "b"_ss);
    });

    // started=true
    dummy_s->send(message::CallResponse{{}, 1, 0, true});
    wcli_->waitRecv();
    EXPECT_TRUE(r.started.get());
    // return error
    dummy_s->send(message::CallResult{{}, 1, 0, true, ValAdaptor("a")});
    wcli_->waitRecv();
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
    dummy_s->waitRecv<message::Call>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 2);
        EXPECT_EQ(obj.target_member_id, 10);
        EXPECT_EQ(obj.field, "b"_ss);
    });

    // started=true
    dummy_s->send(message::CallResponse{{}, 2, 0, true});
    wcli_->waitRecv();
    // return
    dummy_s->send(message::CallResult{{}, 2, 0, false, ValAdaptor("b")});
    wcli_->waitRecv();
    EXPECT_EQ(static_cast<std::string>(r.result.get()), "b");
}
TEST_F(ClientTest, funcResponse) {
    dummy_s = std::make_shared<DummyServer>(false);
    auto main_id = std::this_thread::get_id();
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->func("a").set([&](int a) {
        EXPECT_EQ(std::this_thread::get_id(), main_id);
        if (a == 0) {
            throw std::invalid_argument("a==0");
        } else {
            return std::async(std::launch::deferred, [&, a] {
                EXPECT_NE(std::this_thread::get_id(), main_id);
                if (a == 1) {
                    throw std::invalid_argument("a==1");
                } else {
                    return a;
                }
            });
        }
    });
    // not found
    dummy_s->send(message::Call{7, 100, 0, "n"_ss, {}});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 7);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.started, false);
    });
    dummy_s->recvClear();

    // arg error
    dummy_s->send(
        message::Call{8, 100, 0, "a"_ss, {ValAdaptor(1), ValAdaptor("zzz")}});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 8);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 8);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.is_error, true);
        EXPECT_EQ(static_cast<std::string>(obj.result),
                  "requires 1 arguments, got 2");
    });
    dummy_s->recvClear();

    // throw
    dummy_s->send(message::Call{9, 100, 0, "a"_ss, {ValAdaptor(0)}});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 9);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 9);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.is_error, true);
        // 関数の中でthrowされた内容
        EXPECT_EQ(static_cast<std::string>(obj.result), "a==0");
    });
    dummy_s->recvClear();

    // throw
    dummy_s->send(message::Call{10, 100, 0, "a"_ss, {ValAdaptor(1)}});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 10);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 10);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.is_error, true);
        // 関数の中でthrowされた内容
        EXPECT_EQ(static_cast<std::string>(obj.result), "a==1");
    });
    dummy_s->recvClear();

    // success
    dummy_s->send(message::Call{19, 100, 0, "a"_ss, {ValAdaptor(123)}});
    wcli_->waitRecv();
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 19);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 19);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.is_error, false);
        // 関数の中でthrowされた内容
        EXPECT_EQ(static_cast<int>(obj.result), 123);
    });
    dummy_s->recvClear();
}
