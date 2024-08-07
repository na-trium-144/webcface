#include "client_test.h"

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
                    LogLineData{
                        0, std::chrono::system_clock::now(),
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
    EXPECT_EQ(data_->log_store.getRecv("a"_ss)
                  .value()
                  ->at(0)
                  .message_.u8String()
                  .size(),
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
