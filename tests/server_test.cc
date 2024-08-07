#include "server_test.h"

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
    dummy_c3 = std::make_shared<DummyClient>();
    wait();
    dummy_c1->send(message::SyncInit{{}, ""_ss, 0, "", "", ""});
    dummy_c2->send(message::SyncInit{{}, "c2"_ss, 0, "a", "1", ""});
    dummy_c2->send(message::Text{{}, "a"_ss, std::make_shared<ValAdaptor>("")});
    dummy_c1->waitRecv<message::SyncInit>([&](const auto &obj) {
        EXPECT_EQ(obj.member_name, "c2"_ss);
        EXPECT_EQ(obj.member_id, 2);
        EXPECT_EQ(obj.lib_name, "a");
        EXPECT_EQ(obj.lib_ver, "1");
        EXPECT_EQ(obj.addr, "127.0.0.1");
    });
    dummy_c1->waitRecv<message::SyncInitEnd>([&](const auto &obj) {
        EXPECT_EQ(obj.svr_name, WEBCFACE_SERVER_NAME);
        EXPECT_EQ(obj.ver, WEBCFACE_VERSION);
        EXPECT_EQ(obj.member_id, 1);
        EXPECT_FALSE(obj.hostname.empty());
    });
    dummy_c2->waitRecv<message::SyncInitEnd>([&](const auto &obj) {
        EXPECT_EQ(obj.svr_name, WEBCFACE_SERVER_NAME);
        EXPECT_EQ(obj.ver, WEBCFACE_VERSION);
        EXPECT_EQ(obj.member_id, 2);
        EXPECT_FALSE(obj.hostname.empty());
    });
    dummy_c2->recv<message::SyncInit>(
        [&](auto) { ADD_FAILURE() << "should not receive syncinit"; },
        [&]() {});

    dummy_c3->send(message::SyncInit{{}, "c3"_ss, 0, "a", "1", ""});
    dummy_c3->waitRecv<message::SyncInit>([&](const auto &) {});
    dummy_c3->waitRecv<message::SyncInitEnd>([&](const auto &obj) {
        EXPECT_EQ(obj.svr_name, WEBCFACE_SERVER_NAME);
        EXPECT_EQ(obj.ver, WEBCFACE_VERSION);
        EXPECT_EQ(obj.member_id, 3);
        EXPECT_FALSE(obj.hostname.empty());
    });
    dummy_c3->recv<message::Entry<message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member_id, 2);
            EXPECT_EQ(obj.field, "a"_ss);
        },
        [&] { ADD_FAILURE() << "should have been received entry"; });

    ASSERT_TRUE(server->store->clients_by_id.count(1));
    ASSERT_TRUE(server->store->clients_by_id.count(2));
    ASSERT_TRUE(server->store->clients_by_id.count(3));
    EXPECT_EQ(server->store->clients_by_id.at(1)->name, ""_ss);
    EXPECT_EQ(server->store->clients_by_id.at(2)->name, "c2"_ss);
    EXPECT_EQ(server->store->clients_by_id.at(3)->name, "c3"_ss);
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
                   LogLineData{0, std::chrono::system_clock::now(), "0"_ss}
                       .toMessage(),
                   LogLineData{1, std::chrono::system_clock::now(), "1"_ss}
                       .toMessage(),
                   LogLineData{2, std::chrono::system_clock::now(), "2"_ss}
                       .toMessage(),
                   LogLineData{3, std::chrono::system_clock::now(), "3"_ss}
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
                   LogLineData{4, std::chrono::system_clock::now(), "4"_ss}
                       .toMessage(),
                   LogLineData{5, std::chrono::system_clock::now(), "5"_ss}
                       .toMessage(),
                   LogLineData{6, std::chrono::system_clock::now(), "6"_ss}
                       .toMessage(),
                   LogLineData{7, std::chrono::system_clock::now(), "7"_ss}
                       .toMessage(),
                   LogLineData{8, std::chrono::system_clock::now(), "8"_ss}
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
