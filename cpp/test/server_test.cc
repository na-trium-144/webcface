#include <gtest/gtest.h>
#include "../server/websock.h"
#include "../server/store.h"
#include "../server/s_client_data.h"
#include "../message/message.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace WebCFace;

static void wait(int ms = 10) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class ServerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        Server::store.clear();
        auto stderr_sink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        server_thread = std::make_shared<std::thread>(
            Server::serverRun, 27530, stderr_sink, spdlog::level::trace);
        wait();
        dummy_c = std::make_shared<DummyClient>();
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown" << std::endl;
        Server::serverStop();
        server_thread->join();
    }
    std::shared_ptr<std::thread> server_thread;
    std::shared_ptr<DummyClient> dummy_c;
    int callback_called;
};

TEST_F(ServerTest, connection) { EXPECT_EQ(Server::store.clients.size(), 1); }
