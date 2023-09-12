#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/client.h>
#include "../message/message.h"
#include <spdlog/logger.h>
#include <cinatra.hpp>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <deque>

using namespace WebCFace;

std::shared_ptr<cinatra::connection<cinatra::NonSSL>> connPtr;
std::deque<std::pair<int, std::any>> test_server_recv;

// clientから受信したメッセージがT型であればf1, そうでなければf2を実行する
template <typename T, typename F1, typename F2>
void testServerRecv(const F1 &on_ok, const F2 &on_ng) {
    while (!test_server_recv.empty()) {
        auto m = test_server_recv.front();
        test_server_recv.pop_front();
        if (m.first != -1) {
            if (m.first == T::kind) {
                try {
                    on_ok(std::any_cast<T>(m.second));
                } catch (std::bad_cast) {
                    on_ng(T::kind, m.first);
                }
            } else {
                on_ng(T::kind, m.first);
            }
            return;
        }
    }
    on_ng(T::kind, 0);
}
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

class ClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        static bool is_first = true;
        static std::thread test_server_thread{testServerRun};
        if (is_first) {
            test_server_thread.detach();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            is_first = false;
        }
        data_ = std::make_shared<ClientData>(self_name);
        wcli_ = std::make_shared<Client>(self_name, "127.0.0.1", 17530, data_);
        callback_called = 0;
        // 接続を待機する (todo: 接続完了まで待機する関数があると良い)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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
TEST_F(ClientTest, sync) {
    wcli_->sync();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    using namespace WebCFace::Message;
    auto on_fail = [&](auto ex, auto ac) {
        ADD_FAILURE() << "expected kind: " << ex << ", actual: " << ac;
    };
    testServerRecv<SyncInit>(
        [&](const auto &obj) { EXPECT_EQ(obj.member_name, self_name); },
        on_fail);
    testServerRecv<Sync>([&](const auto &) {}, on_fail);
}