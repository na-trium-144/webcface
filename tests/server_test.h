#pragma once
#include "test_common.h"
#include "dummy_client.h"
#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/server/server.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <webcface/view.h>
#include <webcface/image.h>
#include <webcface/robot_model.h>
#include <webcface/canvas3d.h>
#include <webcface/canvas2d.h>
#include <iostream>

class ServerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        server = std::make_unique<server::Server>(27530, 0);
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override {
        std::cout << "TearDown begin" << std::endl;
        dummy_c1.reset();
        dummy_c2.reset();
        dummy_c3.reset();
        server.reset();
        std::cout << "TearDown end" << std::endl;
    }
    std::unique_ptr<server::Server> server;
    std::shared_ptr<internal::ClientData> data_ =
        std::make_shared<internal::ClientData>("a"_ss);
    std::shared_ptr<DummyClient> dummy_c1, dummy_c2, dummy_c3;
    int callback_called;
};
