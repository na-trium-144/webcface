#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/client.h>
#include <webcface/logger.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include "../message/message.h"
#include <chrono>
#include <thread>
#include <iostream>
#include "dummy_server.h"

using namespace WebCFace;

void wait(int ms = 10) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

class ClientTest : public ::testing::Test {
  protected:
    void SetUp() override {
        std::cout << "SetUp begin" << std::endl;
        dummy_s = std::make_shared<DummyServer>();
        wait();
        WebCFace::logger_internal_level = spdlog::level::trace;
        data_ = std::make_shared<ClientData>(self_name);
        wcli_ = std::make_shared<Client>(self_name, "127.0.0.1", 17530, data_);
        callback_called = 0;
        dummy_s->recvClear();
        // 接続を待機する (todo: 接続完了まで待機する関数があると良い)
        wait();
        std::cout << "SetUp end" << std::endl;
    }
    void TearDown() override { std::cout << "TearDown" << std::endl; }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
    std::shared_ptr<Client> wcli_;
    std::shared_ptr<DummyServer> dummy_s;
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(ClientTest, connection) {
    EXPECT_TRUE(dummy_s->connected());
    EXPECT_TRUE(wcli_->connected());
}
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
    dummy_s->recv<SyncInit>(
        [&](const auto &obj) { EXPECT_EQ(obj.member_name, self_name); },
        [&] { ADD_FAILURE() << "SyncInit recv error"; });
    dummy_s->recv<Sync>([&](const auto &) {},
                        [&] { ADD_FAILURE() << "Sync recv error"; });

    dummy_s->recvClear();
    wcli_->sync();
    wait();
    dummy_s->recv<Sync>([&](const auto &) {},
                        [&] { ADD_FAILURE() << "Sync recv error"; });
}
TEST_F(ClientTest, valueSend) {
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
}
TEST_F(ClientTest, valueReq) {
    data_->value_store.getRecv("a", "b");
    wcli_->sync();
    wait();
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
    data_->text_store.setSend("a", std::make_shared<std::string>("b"));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Text>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(*obj.data, "b");
        },
        [&] { ADD_FAILURE() << "Text recv error"; });
}
TEST_F(ClientTest, textReq) {
    data_->text_store.getRecv("a", "b");
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Req<Message::Text>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "Text Req recv error"; });
    dummy_s->send(
        Message::Res<Message::Text>{1, "", std::make_shared<std::string>("z")});
    dummy_s->send(Message::Res<Message::Text>{
        1, "c", std::make_shared<std::string>("z")});
    wait();
    EXPECT_TRUE(data_->text_store.getRecv("a", "b").has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a", "b").value(), "z");
    EXPECT_TRUE(data_->text_store.getRecv("a", "b.c").has_value());
    EXPECT_EQ(*data_->text_store.getRecv("a", "b.c").value(), "z");
}
TEST_F(ClientTest, viewSend) {
    data_->view_store.setSend(
        "a", std::make_shared<std::vector<ViewComponentBase>>(
                 std::vector<ViewComponentBase>{
                     ViewComponents::text("a")
                         .textColor(ViewColor::yellow)
                         .bgColor(ViewColor::green),
                     ViewComponents::newLine(),
                     ViewComponents::button("a", Func{Field{data_, "x", "y"}}),
                 }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::View>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            EXPECT_EQ(obj.data_diff->size(), 3);
            EXPECT_EQ((*obj.data_diff)[0].type, ViewComponentType::text);
            EXPECT_EQ((*obj.data_diff)[0].text, "a");
            EXPECT_EQ((*obj.data_diff)[0].text_color, ViewColor::yellow);
            EXPECT_EQ((*obj.data_diff)[0].bg_color, ViewColor::green);
            EXPECT_EQ((*obj.data_diff)[1].type, ViewComponentType::new_line);
            EXPECT_EQ((*obj.data_diff)[2].type, ViewComponentType::button);
            EXPECT_EQ((*obj.data_diff)[2].text, "a");
            EXPECT_EQ((*obj.data_diff)[2].on_click_member, "x");
            EXPECT_EQ((*obj.data_diff)[2].on_click_field, "y");
        },
        [&] { ADD_FAILURE() << "View recv error"; });
    dummy_s->recvClear();

    data_->view_store.setSend(
        "a", std::make_shared<std::vector<ViewComponentBase>>(
                 std::vector<ViewComponentBase>{
                     ViewComponents::text("b")
                         .textColor(ViewColor::red)
                         .bgColor(ViewColor::green),
                     ViewComponents::newLine(),
                     ViewComponents::button("a", Func{Field{data_, "x", "y"}}),
                 }));
    wcli_->sync();
    wait();
    dummy_s->recv<Message::View>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.field, "a");
            EXPECT_EQ(obj.length, 3);
            EXPECT_EQ(obj.data_diff->size(), 1);
            EXPECT_EQ((*obj.data_diff)[0].type, ViewComponentType::text);
            EXPECT_EQ((*obj.data_diff)[0].text, "b");
            EXPECT_EQ((*obj.data_diff)[0].text_color, ViewColor::red);
            EXPECT_EQ((*obj.data_diff)[0].bg_color, ViewColor::green);
        },
        [&] { ADD_FAILURE() << "View recv error"; });
}
TEST_F(ClientTest, viewReq) {
    data_->view_store.getRecv("a", "b");
    wcli_->sync();
    wait();
    dummy_s->recv<Message::Req<Message::View>>(
        [&](const auto &obj) {
            EXPECT_EQ(obj.member, "a");
            EXPECT_EQ(obj.field, "b");
            EXPECT_EQ(obj.req_id, 1);
        },
        [&] { ADD_FAILURE() << "View Req recv error"; });
    // dummy_s->send(
    //     Message::Res<Message::View>{1, "",
    //     std::make_shared<std::string>("z")});
    // dummy_s->send(Message::Res<Message::Text>{
    //     1, "c", std::make_shared<std::string>("z")});
    // wait();
    // EXPECT_TRUE(data_->text_store.getRecv("a", "b").has_value());
    // EXPECT_EQ(*data_->text_store.getRecv("a", "b").value(), "z");
    // EXPECT_TRUE(data_->text_store.getRecv("a", "b.c").has_value());
    // EXPECT_EQ(*data_->text_store.getRecv("a", "b.c").value(), "z");
}