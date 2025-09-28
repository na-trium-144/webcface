#include "test_common.h"
#include "client_test.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/common/internal/message/robot_model.h"
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/view.h"
#include <future>

TEST_F(ClientTest, unixSocketConnection) {
    auto dummy_tcp_s = std::make_shared<DummyServer>(false);
    dummy_s = std::make_shared<DummyServer>(true);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    EXPECT_FALSE(Member(*wcli_).connected());
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    EXPECT_TRUE(Member(*wcli_).connected());
}
TEST_F(ClientTest, connectionByStart) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    EXPECT_FALSE(Member(*wcli_).connected());
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    EXPECT_TRUE(Member(*wcli_).connected());
}
TEST_F(ClientTest, connectionByWait) {
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->onConnect([&] { ++callback_called; });
    std::promise<void> p;
    auto f = p.get_future();
    std::thread t([&] {
        wcli_->waitConnection();
        p.set_value();
    });
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    EXPECT_EQ(callback_called, 0);
    dummy_s->waitRecv<message::SyncInit>([&](const auto &) {});
    EXPECT_NE(f.wait_for(std::chrono::milliseconds(0)),
              std::future_status::ready);
    EXPECT_EQ(callback_called, 0);
    dummy_s->send(message::SyncInitEnd{{}, "", "", 0, ""});
    t.join();
    f.get();
    EXPECT_TRUE(wcli_->connected());
    EXPECT_EQ(callback_called, 1);
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
    dummy_s->waitRecv<message::SyncInit>([&](const auto &) {});
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
TEST_F(ClientTest, close) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    EXPECT_EQ(callback_called, 0);
    wcli_.reset();
    wait();
    EXPECT_FALSE(dummy_s->connected());
}
TEST_F(ClientTest, disconnect) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->onDisconnect([&] { ++callback_called; });
    wcli_->autoReconnect(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    EXPECT_EQ(callback_called, 0);
    dummy_s.reset();
    wait();
    wcli_->sync();
    wait();
    wcli_->sync();
    EXPECT_FALSE(wcli_->connected());
    EXPECT_EQ(callback_called, 1);
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

    dummy_s->send(message::SyncInitEnd{{}, "a", "1", 0, "b"});
    wait();
    EXPECT_EQ(wcli_->serverName(), "");
    wcli_->sync();
    EXPECT_EQ(wcli_->serverName(), "a");
    EXPECT_EQ(wcli_->serverVersion(), "1");
    EXPECT_EQ(wcli_->serverHostName(), "b");
}
TEST_F(ClientTest, syncThread) {
    wcli_->autoReconnect(false);
    wcli_->start();
    // 未接続&autoReconnectがfalseなら即return
    wcli_->loopSync();

    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    auto main_id = std::this_thread::get_id();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").value("b").onChange([&](const Value &) {
        EXPECT_EQ(std::this_thread::get_id(), main_id);
        callback_called++;
    });
    wait();
    dummy_s->send(message::Res<message::Value>{
        1, ""_ss, MutableNumVector(std::vector<double>{1, 2, 3})});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
}
// TEST_F(ClientTest, autoSyncThread) {
//     dummy_s = std::make_shared<DummyServer>(false);
//     auto main_id = std::this_thread::get_id();
//     wait();
//     wcli_->autoSync(true);
//     wcli_->start();
//     while (!dummy_s->connected() || !wcli_->connected()) {
//         wait();
//     }
//     wcli_->member("a").value("b").onChange([&](const Value &) {
//         EXPECT_NE(std::this_thread::get_id(), main_id);
//         callback_called++;
//     });
//     wait();
//     dummy_s->send(message::Res<message::Value>{
//         1, ""_ss,
//         std::make_shared<std::vector<double>>(std::vector<double>{1, 2,
//         3})});
//     wait();
//     EXPECT_EQ(callback_called, 1);
// }
TEST_F(ClientTest, syncTimeout) {
    auto start = std::chrono::steady_clock::now();
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    auto end = std::chrono::steady_clock::now();
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.9);
    start = std::chrono::steady_clock::now();
    wcli_->sync();
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
    start = std::chrono::steady_clock::now();
    wcli_->loopSyncFor(std::chrono::milliseconds(-WEBCFACE_TEST_TIMEOUT));
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
}
TEST_F(ClientTest, syncUntilTimeout) {
    auto start = std::chrono::steady_clock::now();
    wcli_->loopSyncUntil(start +
                         std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    auto end = std::chrono::steady_clock::now();
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.9);
    start = std::chrono::steady_clock::now();
    wcli_->loopSyncUntil(start);
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
    start = std::chrono::steady_clock::now();
    wcli_->loopSyncUntil(start -
                         std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    end = std::chrono::steady_clock::now();
    EXPECT_LE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count(),
              WEBCFACE_TEST_TIMEOUT * 0.1);
}

TEST_F(ClientTest, ping) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    dummy_s->waitRecv<message::SyncInit>([&](const auto &) {});
    dummy_s->send(message::SyncInitEnd{{}, "", "", 42, ""});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Ping{});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->waitRecv<message::Ping>([&](const auto &) {});

    wcli_->member("a").onPing(callback<Member>());
    wcli_->onPing(callback<Member>());
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "", "", ""});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::PingStatus{
        {},
        std::make_shared<std::unordered_map<unsigned int, int>>(
            std::unordered_map<unsigned int, int>{{10, 15}, {42, 99}})});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
    wcli_->member("a").onConnect(callback<Member>());
    EXPECT_FALSE(wcli_->member("a").connected());
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "b", "1", "12345"});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_TRUE(wcli_->member("a").connected());
    callback_called = 0;
    EXPECT_EQ(wcli_->members().size(), 1u);
    EXPECT_EQ(wcli_->members()[0].name(), "a");
    EXPECT_EQ(wcli_->members()[0].nameW(), L"a");
    EXPECT_EQ(data_->member_ids.lock().get()["a"_ss], 10u);

    auto m = wcli_->member("a");
    EXPECT_EQ(m.libName(), "b");
    EXPECT_EQ(m.libVersion(), "1");
    EXPECT_EQ(m.remoteAddr(), "12345");

    EXPECT_FALSE(m.value("b").exists());
    m.onValueEntry(callback<Value>());
    dummy_s->send(message::Entry<message::Value>{{}, 10, "b"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.value("b").exists());
    ASSERT_EQ(m.valueEntries().size(), 1u);
    EXPECT_EQ(m.valueEntries()[0].name(), "b");
    EXPECT_EQ(m.valueEntries()[0].nameW(), L"b");

    EXPECT_FALSE(m.text("c").exists());
    m.onTextEntry(callback<Text>());
    dummy_s->send(message::Entry<message::Text>{{}, 10, "c"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.text("c").exists());
    ASSERT_EQ(m.textEntries().size(), 1u);
    EXPECT_EQ(m.textEntries()[0].name(), "c");
    EXPECT_EQ(m.textEntries()[0].nameW(), L"c");

    EXPECT_FALSE(m.view("d").exists());
    m.onViewEntry(callback<View>());
    dummy_s->send(message::Entry<message::View>{{}, 10, "d"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.view("d").exists());
    ASSERT_EQ(m.viewEntries().size(), 1u);
    EXPECT_EQ(m.viewEntries()[0].name(), "d");
    EXPECT_EQ(m.viewEntries()[0].nameW(), L"d");

    EXPECT_FALSE(m.canvas2D("d").exists());
    m.onCanvas2DEntry(callback<Canvas2D>());
    dummy_s->send(message::Entry<message::Canvas2D>{{}, 10, "d"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.canvas2D("d").exists());
    ASSERT_EQ(m.canvas2DEntries().size(), 1u);
    EXPECT_EQ(m.canvas2DEntries()[0].name(), "d");
    EXPECT_EQ(m.canvas2DEntries()[0].nameW(), L"d");

    EXPECT_FALSE(m.canvas3D("d").exists());
    m.onCanvas3DEntry(callback<Canvas3D>());
    dummy_s->send(message::Entry<message::Canvas3D>{{}, 10, "d"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.canvas3D("d").exists());
    ASSERT_EQ(m.canvas3DEntries().size(), 1u);
    EXPECT_EQ(m.canvas3DEntries()[0].name(), "d");
    EXPECT_EQ(m.canvas3DEntries()[0].nameW(), L"d");

    EXPECT_FALSE(m.robotModel("d").exists());
    m.onRobotModelEntry(callback<RobotModel>());
    dummy_s->send(message::Entry<message::RobotModel>{{}, 10, "d"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.robotModel("d").exists());
    ASSERT_EQ(m.robotModelEntries().size(), 1u);
    EXPECT_EQ(m.robotModelEntries()[0].name(), "d");
    EXPECT_EQ(m.robotModelEntries()[0].nameW(), L"d");

    EXPECT_FALSE(m.image("d").exists());
    m.onImageEntry(callback<Image>());
    dummy_s->send(message::Entry<message::Image>{{}, 10, "d"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.image("d").exists());
    ASSERT_EQ(m.imageEntries().size(), 1u);
    EXPECT_EQ(m.imageEntries()[0].name(), "d");
    EXPECT_EQ(m.imageEntries()[0].nameW(), L"d");

    EXPECT_FALSE(m.func("a").exists());
    m.onFuncEntry(callback<Func>());
    dummy_s->send(message::FuncInfo{
        10, "a"_ss, ValType::int_, {std::make_shared<message::Arg>()}, 5});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.func("a").exists());
    EXPECT_EQ(m.func("a").index(), 5);
    EXPECT_EQ(m.funcEntries().size(), 1u);
    EXPECT_EQ(m.funcEntries()[0].name(), "a");
    EXPECT_EQ(m.funcEntries()[0].nameW(), L"a");

    EXPECT_FALSE(m.log("a").exists());
    EXPECT_FALSE(m.log().exists());
    m.onLogEntry(callback<Log>());
    dummy_s->send(message::Entry<message::Log>{{}, 10, "a"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    EXPECT_TRUE(m.log("a").exists());
    EXPECT_FALSE(m.log().exists());

    m.onSync(callback<Member>());
    dummy_s->send(message::Sync{10, std::chrono::system_clock::now()});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;

    wcli_->member("a").onDisconnect(callback<Member>());
    EXPECT_TRUE(wcli_->member("a").connected());
    dummy_s->send(message::Closed{{}, 10});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_FALSE(wcli_->member("a").connected());
    callback_called = 0;
}
TEST_F(ClientTest, childrenEntry) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "b", "1", "12345"});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    auto m = wcli_->member("a");
    EXPECT_FALSE(m.hasChildren());
    EXPECT_FALSE(m.child("c").hasChildren());

    dummy_s->send(message::Entry<message::Value>{{}, 10, "a"_ss});
    dummy_s->send(message::Entry<message::Text>{{}, 10, "b"_ss});
    dummy_s->send(message::Entry<message::Text>{{}, 10, "c.a"_ss});
    dummy_s->send(message::Entry<message::Text>{{}, 10, "c.b"_ss});
    dummy_s->send(message::Entry<message::Text>{{}, 10, "c.c"_ss});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));

    EXPECT_TRUE(m.hasChildren());
    EXPECT_FALSE(m.child("a").hasChildren());
    EXPECT_FALSE(m.child("b").hasChildren());
    EXPECT_TRUE(m.child("c").hasChildren());
    auto children = m.children();
    EXPECT_EQ(children.size(), 3u);
    EXPECT_EQ(children.at(0).member().name(), "a");
    EXPECT_EQ(children.at(0).name(), "a");
    EXPECT_EQ(children.at(1).member().name(), "a");
    EXPECT_EQ(children.at(1).name(), "b");
    EXPECT_EQ(children.at(2).member().name(), "a");
    EXPECT_EQ(children.at(2).name(), "c");

    auto children_recurse = m.childrenRecurse();
    EXPECT_EQ(children_recurse.size(), 5u);
    EXPECT_EQ(children_recurse.at(0).member().name(), "a");
    EXPECT_EQ(children_recurse.at(0).name(), "a");
    EXPECT_EQ(children_recurse.at(1).name(), "b");
    EXPECT_EQ(children_recurse.at(2).name(), "c.a");
    EXPECT_EQ(children_recurse.at(3).name(), "c.b");
    EXPECT_EQ(children_recurse.at(4).name(), "c.c");

    auto children_a = m.child("a").children();
    EXPECT_EQ(children_a.size(), 0u);

    auto children_c = m.child("c").children();
    EXPECT_EQ(children_c.size(), 3u);
    EXPECT_EQ(children_c.at(0).member().name(), "a");
    EXPECT_EQ(children_c.at(0).name(), "c.a");
    EXPECT_EQ(children_c.at(1).member().name(), "a");
    EXPECT_EQ(children_c.at(1).name(), "c.b");
    EXPECT_EQ(children_c.at(2).member().name(), "a");
    EXPECT_EQ(children_c.at(2).name(), "c.c");
}

TEST_F(ClientTest, logSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto ls = std::make_shared<internal::LogData>(std::deque<LogLineData>{
        {0, std::chrono::system_clock::now(),
         SharedString::fromU8String(std::string(100000, 'a'))},
        {1, std::chrono::system_clock::now(), "b"_ss},
    });
    data_->log_store.setSend("a"_ss, ls);
    wcli_->sync();
    dummy_s->waitRecv<message::Log>([&](const auto &obj) {
        EXPECT_EQ(obj.field, "a"_ss);
        EXPECT_EQ(obj.log->size(), 2u);
        EXPECT_EQ(obj.log->at(0).level_, 0);
        EXPECT_EQ(obj.log->at(0).message_.decode().size(), 100000u);
        EXPECT_EQ(obj.log->at(1).level_, 1);
        EXPECT_EQ(obj.log->at(1).message_, "b"_ss);
    });

    dummy_s->recvClear();
    ls->data.push_back(
        LogLineData{2, std::chrono::system_clock::now(), "c"_ss});
    data_->log_store.setSend("a"_ss, ls);
    wcli_->sync();
    dummy_s->waitRecv<message::Log>([&](const auto &obj) {
        EXPECT_EQ(obj.log->size(), 1u);
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
    wcli_->member("a").log("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Log>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8StringView(), "a");
        EXPECT_EQ(obj.field.u8StringView(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    wcli_->member("a").log("b").onChange(callback<Log>());

    dummy_s->send(message::SyncInit{{}, "a"_ss, 10, "", "", ""});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Log>{
        1, ""_ss,
        std::make_shared<std::deque<message::LogLine>>(
            std::deque<message::LogLine>{
                LogLineData{
                    0, std::chrono::system_clock::now(),
                    SharedString::fromU8String(std::string(100000, 'a'))}
                    .toMessage(),
                LogLineData{1, std::chrono::system_clock::now(), "b"_ss}
                    .toMessage(),
            })});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->log_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.size(),
              2u);
    EXPECT_EQ(
        data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.at(0).level_, 0);
    EXPECT_EQ(data_->log_store.getRecv("a"_ss, "b"_ss)
                  .value()
                  ->data.at(0)
                  .message_.u8StringView()
                  .size(),
              100000u);

    dummy_s->send(message::Res<message::Log>{
        1, ""_ss,
        std::make_shared<std::deque<message::LogLine>>(
            std::deque<message::LogLine>{
                LogLineData{2, std::chrono::system_clock::now(), "c"_ss}
                    .toMessage(),
            })});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_TRUE(data_->log_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.size(),
              3u);

    // keep_lines以上のログは保存しない
    webcface::Log::keepLines(2);
    dummy_s->send(message::Res<message::Log>{
        1, ""_ss,
        std::make_shared<std::deque<message::LogLine>>(
            std::deque<message::LogLine>{
                LogLineData{3, std::chrono::system_clock::now(), "d"_ss}
                    .toMessage(),
            })});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 3);
    EXPECT_TRUE(data_->log_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.size(),
              2u);
    EXPECT_EQ(
        data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.at(0).level_, 2);
    EXPECT_EQ(
        data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.at(1).level_, 3);

    dummy_s->send(message::Res<message::Log>{
        1, ""_ss,
        std::make_shared<std::deque<message::LogLine>>(
            std::deque<message::LogLine>{
                LogLineData{4, std::chrono::system_clock::now(), "d"_ss}
                    .toMessage(),
                LogLineData{5, std::chrono::system_clock::now(), "d"_ss}
                    .toMessage(),
                LogLineData{6, std::chrono::system_clock::now(), "d"_ss}
                    .toMessage(),
                LogLineData{7, std::chrono::system_clock::now(), "d"_ss}
                    .toMessage(),
            })});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 4);
    EXPECT_TRUE(data_->log_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.size(),
              2u);
    EXPECT_EQ(
        data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.at(0).level_, 6);
    EXPECT_EQ(
        data_->log_store.getRecv("a"_ss, "b"_ss).value()->data.at(1).level_, 7);

    webcface::Log::keepLines(1000);
}
