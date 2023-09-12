#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/client.h>
#include <webcface/logger.h>
#include "../message/message.h"
#include <spdlog/logger.h>
#include <cinatra.hpp>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <vector>

using namespace WebCFace;

std::shared_ptr<cinatra::connection<cinatra::NonSSL>> connPtr;
std::vector<std::pair<int, std::any>> test_server_recv;

// clientから受信したメッセージがT型であればf1, そうでなければf2を実行する
template <typename T, typename F1, typename F2>
void testServerRecv(const F1 &on_ok, const F2 &on_ng) {
    for (const auto &m : test_server_recv) {
        if (m.first == T::kind) {
            on_ok(std::any_cast<T>(m.second));
            return;
        }
    }
    on_ng();
}
void testServerRecvClear() { test_server_recv.clear(); }

// clientにメッセージを送信する
template <typename T>
void testServerSend(const T &msg) {
    connPtr->send_ws_binary(Message::packSingle(msg));
}
auto dummy_logger = std::make_shared<spdlog::logger>("test");

void testServerRun() {
    using namespace cinatra;

    http_server server(1);
    server.listen("0.0.0.0", "17530");
    server.set_http_handler<GET, POST>("/", [](request &req, response &res) {
        req.on(ws_open,
               [](request &req) { connPtr = req.get_conn<cinatra::NonSSL>(); });
        req.on(ws_message, [](request &req) {
            auto part_data = req.get_part_data();
            std::string str = std::string(part_data.data(), part_data.length());
            auto unpacked = Message::unpack(str, dummy_logger);
            for (const auto &m : unpacked) {
                test_server_recv.push_back(m);
            }
        });
    });
    server.run();
}

void wait(int ms = 10) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class ClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        WebCFace::logger_internal_level = spdlog::level::trace;
        static bool is_first = true;
        static std::thread test_server_thread{testServerRun};
        if (is_first) {
            test_server_thread.detach();
            wait();
            is_first = false;
        }
        data_ = std::make_shared<ClientData>(self_name);
        wcli_ = std::make_shared<Client>(self_name, "127.0.0.1", 17530, data_);
        callback_called = 0;
        testServerRecvClear();
        // 接続を待機する (todo: 接続完了まで待機する関数があると良い)
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override { std::cout << "TearDown" << std::endl; }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
    std::shared_ptr<Client> wcli_;
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(ClientTest, connection) { EXPECT_NE(connPtr, nullptr); }
TEST_F(ClientTest, name) { EXPECT_EQ(wcli_->name(), self_name); }
TEST_F(ClientTest, memoryLeak) {
    wcli_.reset();
    wait(300);
    EXPECT_EQ(data_.use_count(), 1);
}
TEST_F(ClientTest, sync) {
    wcli_->sync();
    wait();
    using namespace WebCFace::Message;
    testServerRecv<SyncInit>(
        [&](const auto &obj) { EXPECT_EQ(obj.member_name, self_name); },
        [&] { ADD_FAILURE() << "SyncInit recv error"; });
    testServerRecv<Sync>([&](const auto &) {},
                         [&] { ADD_FAILURE() << "Sync recv error"; });
}
TEST_F(ClientTest, valueSend) {
    data_->value_store.setSend("a", std::make_shared<VectorOpt<double>>(5));
    wcli_->sync();
    wait();
    testServerRecv<Message::Value>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.data->size(), 1);
            EXPECT_EQ(obj.data->at(0), 5);
        },
        [&] { ADD_FAILURE() << "Value recv error"; });
}
TEST_F(ClientTest, valueReq) {
    data_->value_store.getRecv("a", "b");
    wcli_->sync();
    wait();
    testServerRecv<Message::Req<Message::Value>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Value Req recv error"; });
    testServerSend(Message::Res<Message::Value>{
        1, "",
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    testServerSend(Message::Res<Message::Value>{
        1, "c",
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wait();
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