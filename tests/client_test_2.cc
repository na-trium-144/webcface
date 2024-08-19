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
        EXPECT_EQ(obj.data->size(), 1);
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
        ASSERT_EQ(obj.data->size(), 2);
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
        EXPECT_EQ(obj.req_id, 1);
    });
    wcli_->member("a").value("b").onChange(callback<Value>());
    dummy_s->send(message::Res<message::Value>{
        1, ""_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Value>{
        1, "c"_ss,
        std::make_shared<std::vector<double>>(std::vector<double>{1, 2, 3})});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b"_ss).value())
                  .size(),
              3);
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b.c"_ss).has_value());
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b.c"_ss).value())
                  .size(),
              3);
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
        EXPECT_EQ(obj.req_id, 1);
    });
    wcli_->member("a").text("b").onChange(callback<Text>());
    dummy_s->send(message::Res<message::Text>{
        1, ""_ss, std::make_shared<ValAdaptor>("z")});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Text>{
        1, "c"_ss, std::make_shared<ValAdaptor>("z")});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
        EXPECT_EQ(obj.length, 3);
        EXPECT_EQ(obj.data_diff->size(), 3);
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
        EXPECT_EQ(obj.length, 3);
        EXPECT_EQ(obj.data_diff->size(), 1);
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
        EXPECT_EQ(obj.req_id, 1);
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
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::View>{1, "c"_ss, v, 3});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->view_store.getRecv("a"_ss, "b"_ss).has_value());
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->size(), 3);
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
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss).value()->size(), 3);
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
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
    ASSERT_FALSE(r.reached());
    ASSERT_FALSE(r.finished());

    // started=false
    dummy_s->send(message::CallResponse{{}, 0, 0, false});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    ASSERT_TRUE(r.reached());
    EXPECT_FALSE(r.found());
    ASSERT_TRUE(r.finished());
    EXPECT_TRUE(r.isError());
    dummy_s->recvClear();

    // 2nd call id=1
    r = wcli_->member("a").func("b").runAsync(1, true, "a");
    dummy_s->waitRecv<message::Call>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 1);
        EXPECT_EQ(obj.target_member_id, 10);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    ASSERT_FALSE(r.reached());
    ASSERT_FALSE(r.finished());

    // started=true
    dummy_s->send(message::CallResponse{{}, 1, 0, true});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    ASSERT_TRUE(r.reached());
    EXPECT_TRUE(r.found());
    // return error
    dummy_s->send(message::CallResult{{}, 1, 0, true, ValAdaptor("a")});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    ASSERT_TRUE(r.finished());
    EXPECT_TRUE(r.isError());
    EXPECT_EQ(r.rejection(), "a");
    dummy_s->recvClear();

    // 3rd call id=2
    r = wcli_->member("a").func("b").runAsync(1, true, "a");
    dummy_s->waitRecv<message::Call>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 2);
        EXPECT_EQ(obj.target_member_id, 10);
        EXPECT_EQ(obj.field, "b"_ss);
    });
    ASSERT_FALSE(r.reached());
    ASSERT_FALSE(r.finished());

    // started=true
    dummy_s->send(message::CallResponse{{}, 2, 0, true});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    ASSERT_TRUE(r.reached());
    // return
    dummy_s->send(message::CallResult{{}, 2, 0, false, ValAdaptor("b")});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    ASSERT_TRUE(r.finished());
    EXPECT_EQ(static_cast<std::string>(r.response()), "b");
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
            return a;
        }
    });
    // not found
    dummy_s->send(message::Call{7, 100, 0, "n"_ss, {}});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 7);
        EXPECT_EQ(obj.caller_member_id, 100);
        EXPECT_EQ(obj.started, false);
    });
    dummy_s->recvClear();

    // arg error
    dummy_s->send(
        message::Call{8, 100, 0, "a"_ss, {ValAdaptor(1), ValAdaptor("zzz")}});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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

    // success
    dummy_s->send(message::Call{19, 100, 0, "a"_ss, {ValAdaptor(123)}});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
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
