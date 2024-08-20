#include "client_test.h"

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
        EXPECT_EQ(obj.data->size(), 1u);
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
        ASSERT_EQ(obj.data->size(), 2u);
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
        EXPECT_EQ(obj.req_id, 1u);
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
              3u);
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b.c"_ss).value())
                  .size(),
              3u);
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
    wcli_->waitRecvUntil(start +
                         std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
    wcli_->waitRecvUntil(start -
                         std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
        EXPECT_EQ(obj.req_id, 1u);
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
        EXPECT_EQ(obj.length, 3u);
        EXPECT_EQ(obj.data_diff->size(), 3u);
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
        EXPECT_EQ(obj.length, 3u);
        EXPECT_EQ(obj.data_diff->size(), 1u);
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
        EXPECT_EQ(obj.req_id, 1u);
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
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->size(), 3u);
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
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->size(), 3u);
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
