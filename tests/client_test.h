#pragma once
#include "test_common.h"
#include "webcface/common/internal/message/pack.h"
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
#include <chrono>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace webcface;

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
