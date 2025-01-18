#include "client_test.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/internal/component_internal.h"

TEST_F(ClientTest, valueSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    data_->value_store.setSend(
        "a"_ss, std::make_shared<std::vector<double>>(std::vector<double>{5}));
    data_->value_store.setEntry(self_name, "a"_ss, {{2}, true});
    wcli_->sync();
    dummy_s->waitRecv<message::Value>([&](const auto &obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data->size(), 1u);
        EXPECT_EQ(obj.data->at(0), 5);
        EXPECT_EQ(obj.shape, std::vector<std::size_t>{2});
        EXPECT_TRUE(obj.fixed);
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
        EXPECT_EQ(obj.field.u8String(), "a");
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
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
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
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b"_ss));
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b"_ss))
                  .size(),
              3u);
    EXPECT_TRUE(data_->value_store.getRecv("a"_ss, "b.c"_ss));
    EXPECT_EQ(static_cast<std::vector<double>>(
                  *data_->value_store.getRecv("a"_ss, "b.c"_ss))
                  .size(),
              3u);
}
TEST_F(ClientTest, valueReqTiming) {
    wcli_->autoReconnect(false);
    // 1. 接続前、sync前
    wcli_->member("a").value("1").request();
    wcli_->sync();
    // 2. 接続前、sync後
    wcli_->member("a").value("2").request();

    wait();
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }

    // 3. 接続後
    wcli_->member("a").value("3").request();

    wait();

    dummy_s.reset();
    while (wcli_->connected()) {
        wait();
    }
    wait();
    // 4. 切断後、sync前
    wcli_->member("a").value("4").request();
    wcli_->sync();
    // 5. 切断後、sync後
    wcli_->member("a").value("5").request();

    wait();
    dummy_s = std::make_shared<DummyServer>(false);
    wait();
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }

    // 6. 再接続後
    wcli_->member("a").value("6").request();
    wait();

    std::array<bool, 6> req_found = {};
    {
        std::lock_guard lock(dummy_s->server_m);
        for (const auto &m : dummy_s->recv_data) {
            if (m.first ==
                message::MessageKind::req + message::MessageKind::value) {
                auto &obj = *static_cast<message::Req<message::Value> *>(
                    m.second.get());
                EXPECT_EQ(obj.member.u8String(), "a");
                ASSERT_GE(obj.req_id, 1u);
                ASSERT_LE(obj.req_id, 6u);
                EXPECT_EQ(obj.field.u8String(), std::to_string(obj.req_id));
                EXPECT_FALSE(req_found.at(obj.req_id - 1));
                req_found.at(obj.req_id - 1) = true;
            }
        }
    }
    EXPECT_EQ(req_found, (std::array<bool, 6>{1, 1, 1, 1, 1, 1}));
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
        EXPECT_EQ(obj.field.u8String(), "a");
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

    for (std::size_t len = 10; len < 1000000; len *= 10) {
        data_->text_store.setSend(
            "a"_ss, std::make_shared<ValAdaptor>(std::string(len, 'a')));
        wcli_->sync();
        dummy_s->waitRecv<message::Text>([&](const auto &obj) {
            EXPECT_EQ(obj.data->asU8StringRef().size(), len);
            for (std::size_t i = 0; i < len; i++) {
                EXPECT_EQ(obj.data->asU8StringRef()[i], 'a');
            }
        });
        dummy_s->recvClear();
    }
}
TEST_F(ClientTest, textSendWithUnix) {
    auto dummy_tcp_s = std::make_shared<DummyServer>(false);
    dummy_s = std::make_shared<DummyServer>(true);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }

    for (std::size_t len = 10; len < 1000000; len *= 10) {
        data_->text_store.setSend(
            "a"_ss, std::make_shared<ValAdaptor>(std::string(len, 'a')));
        wcli_->sync();
        dummy_s->waitRecv<message::Text>([&](const auto &obj) {
            EXPECT_EQ(obj.data->asU8StringRef().size(), len);
            for (std::size_t i = 0; i < len; i++) {
                EXPECT_EQ(obj.data->asU8StringRef()[i], 'a');
            }
        });
        dummy_s->recvClear();
    }
}
TEST_F(ClientTest, textReq) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").text("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    wcli_->member("a").text("b").onChange(callback<Text>());
    dummy_s->send(message::Res<message::Text>{
        1, ""_ss, std::make_shared<ValAdaptor>("z")});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::Text>{
        1, "c"_ss, std::make_shared<ValAdaptor>("z")});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->text_store.getRecv("a"_ss, "b"_ss));
    EXPECT_EQ(*data_->text_store.getRecv("a"_ss, "b"_ss), "z");
    EXPECT_TRUE(data_->text_store.getRecv("a"_ss, "b.c"_ss));
    EXPECT_EQ(*data_->text_store.getRecv("a"_ss, "b.c"_ss), "z");

    for (std::size_t len = 10; len <= WEBCFACE_TEST_MAXLEN; len *= 10) {
        dummy_s->send(message::Res<message::Text>{
            1, ""_ss, std::make_shared<ValAdaptor>(std::string(len, 'a'))});
        while (!data_->text_store.getRecv("a"_ss, "b"_ss) ||
               data_->text_store.getRecv("a"_ss, "b"_ss)
                       ->asU8StringRef()
                       .size() != len) {
            wcli_->loopSyncFor(
                std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
        }
        for (std::size_t i = 0; i < len; i++) {
            EXPECT_EQ(data_->text_store.getRecv("a"_ss, "b"_ss)
                          ->asU8StringRef()[i],
                      'a');
        }
    }
}
TEST_F(ClientTest, textReqWithUnix) {
    auto dummy_tcp_s = std::make_shared<DummyServer>(false);
    dummy_s = std::make_shared<DummyServer>(true);
    wait();
    EXPECT_FALSE(dummy_s->connected());
    EXPECT_FALSE(wcli_->connected());
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    wcli_->member("a").text("b").tryGet();
    dummy_s->waitRecv<message::Req<message::Text>>([&](const auto &obj) {
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });

    for (std::size_t len = 10; len <= WEBCFACE_TEST_MAXLEN; len *= 10) {
        dummy_s->send(message::Res<message::Text>{
            1, ""_ss, std::make_shared<ValAdaptor>(std::string(len, 'a'))});
        while (!data_->text_store.getRecv("a"_ss, "b"_ss) ||
               data_->text_store.getRecv("a"_ss, "b"_ss)
                       ->asU8StringRef()
                       .size() != len) {
            wcli_->loopSyncFor(
                std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
        }
        for (std::size_t i = 0; i < len; i++) {
            EXPECT_EQ(data_->text_store.getRecv("a"_ss, "b"_ss)
                          ->asU8StringRef()[i],
                      'a');
        }
    }
}
TEST_F(ClientTest, viewSend) {
    dummy_s = std::make_shared<DummyServer>(false);
    wcli_->start();
    while (!dummy_s->connected() || !wcli_->connected()) {
        wait();
    }
    auto view_data = std::make_shared<message::ViewData>();
    view_data->components = {
        {"a", std::static_pointer_cast<message::ViewComponentData>(
                  std::shared_ptr(ViewComponents::text("a")
                                      .textColor(ViewColor::yellow)
                                      .bgColor(ViewColor::green)
                                      .component_v.lockTmp(data_, ""_ss)))},
        {"c",
         std::static_pointer_cast<message::ViewComponentData>(std::shared_ptr(
             ViewComponents::button("a", Func{Field{data_, "x"_ss, "y"_ss}})
                 .lockTmp(data_, ""_ss)))},
        {"b",
         std::static_pointer_cast<message::ViewComponentData>(
             std::shared_ptr(ViewComponents::newLine().lockTmp(data_, ""_ss)))},
    };
    view_data->data_ids = {"a"_ss, "b"_ss, "c"_ss};
    data_->view_store.setSend("a"_ss, view_data);
    wcli_->sync();
    dummy_s->waitRecv<message::View>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_NE(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_ids->size(), 3u);
        EXPECT_EQ(obj.data_diff.size(), 3u);
        EXPECT_EQ(obj.data_diff["a"]->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.data_diff["a"]->text, "a"_ss);
        EXPECT_EQ(obj.data_diff["a"]->text_color,
                  static_cast<int>(ViewColor::yellow));
        EXPECT_EQ(obj.data_diff["a"]->bg_color,
                  static_cast<int>(ViewColor::green));
        EXPECT_EQ(obj.data_diff["b"]->type,
                  static_cast<int>(ViewComponentType::new_line));
        EXPECT_EQ(obj.data_diff["c"]->type,
                  static_cast<int>(ViewComponentType::button));
        EXPECT_EQ(obj.data_diff["c"]->text, "a"_ss);
        EXPECT_EQ(obj.data_diff["c"]->on_click_member, "x"_ss);
        EXPECT_EQ(obj.data_diff["c"]->on_click_field, "y"_ss);
    });
    dummy_s->recvClear();

    view_data = std::make_shared<message::ViewData>(*view_data);
    view_data->components = {
        {"a",
         std::static_pointer_cast<message::ViewComponentData>(
             std::shared_ptr(ViewComponents::text("b") // ここのtextと色を変えた
                                 .textColor(ViewColor::red)
                                 .bgColor(ViewColor::green)
                                 .component_v.lockTmp(data_, ""_ss)))},
        {"b",
         std::static_pointer_cast<message::ViewComponentData>(
             std::shared_ptr(ViewComponents::newLine().lockTmp(data_, ""_ss)))},
        {"c",
         std::static_pointer_cast<message::ViewComponentData>(std::shared_ptr(
             ViewComponents::button("a", Func{Field{data_, "x"_ss, "y"_ss}})
                 .lockTmp(data_, ""_ss)))},
    };
    data_->view_store.setSend("a"_ss, view_data);
    wcli_->sync();
    dummy_s->waitRecv<message::View>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_diff.size(), 1u);
        EXPECT_EQ(obj.data_diff["a"]->type,
                  static_cast<int>(ViewComponentType::text));
        EXPECT_EQ(obj.data_diff["a"]->text, "b"_ss);
        EXPECT_EQ(obj.data_diff["a"]->text_color,
                  static_cast<int>(ViewColor::red));
        EXPECT_EQ(obj.data_diff["a"]->bg_color,
                  static_cast<int>(ViewColor::green));
    });
    dummy_s->recvClear();

    view_data = std::make_shared<message::ViewData>(*view_data);
    view_data->data_ids = {"a"_ss, "b"_ss};
    data_->view_store.setSend("a"_ss, view_data);
    wcli_->sync();
    dummy_s->waitRecv<message::View>([&](auto obj) {
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_NE(obj.data_ids, std::nullopt);
        EXPECT_EQ(obj.data_ids->size(), 2u);
        EXPECT_EQ(obj.data_diff.size(), 0u);
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
        EXPECT_EQ(obj.member.u8String(), "a");
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.req_id, 1u);
    });
    wcli_->member("a").view("b").onChange(callback<View>());

    std::map<std::string, std::shared_ptr<message::ViewComponentData>> v{
        {"0", std::static_pointer_cast<message::ViewComponentData>(
                  std::shared_ptr(ViewComponents::text("a")
                                      .textColor(ViewColor::yellow)
                                      .bgColor(ViewColor::green)
                                      .component_v.lockTmp(data_, ""_ss)))},
        {"1",
         std::static_pointer_cast<message::ViewComponentData>(
             std::shared_ptr(ViewComponents::newLine().lockTmp(data_, ""_ss)))},
        {"2",
         std::static_pointer_cast<message::ViewComponentData>(std::shared_ptr(
             ViewComponents::button("a", Func{Field{data_, "x"_ss, "y"_ss}})
                 .lockTmp(data_, ""_ss)))},
    };
    std::vector<SharedString> v_ids = {"0"_ss, "1"_ss, "2"_ss};
    dummy_s->send(message::Res<message::View>{1, ""_ss, v, v_ids});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->send(message::Res<message::View>{1, "c"_ss, v, v_ids});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->view_store.getRecv("a"_ss, "b"_ss));
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)->data_ids.size(), 3u);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)->components.size(), 3u);
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss)->components.at("0")->type,
        static_cast<int>(ViewComponentType::text));
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)
                  ->components.at("0")
                  ->text.u8String(),
              "a");
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)
                  ->components.at("0")
                  ->text_color,
              static_cast<int>(ViewColor::yellow));
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss)->components.at("0")->bg_color,
        static_cast<int>(ViewColor::green));
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss)->components.at("1")->type,
        static_cast<int>(ViewComponentType::new_line));
    EXPECT_TRUE(data_->view_store.getRecv("a"_ss, "b.c"_ss));

    // 差分だけ送る
    std::map<std::string, std::shared_ptr<message::ViewComponentData>> v2{
        {"0", std::static_pointer_cast<message::ViewComponentData>(
                  std::shared_ptr(ViewComponents::text("b")
                                      .textColor(ViewColor::red)
                                      .bgColor(ViewColor::green)
                                      .component_v.lockTmp(data_, ""_ss)))}};
    dummy_s->send(message::Res<message::View>{1, ""_ss, v2, std::nullopt});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)->data_ids.size(), 3u);
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)->components.size(), 3u);
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss)->components.at("0")->type,
        static_cast<int>(ViewComponentType::text));
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)
                  ->components.at("0")
                  ->text.u8String(),
              "b");
    EXPECT_EQ(data_->view_store.getRecv("a"_ss, "b"_ss)
                  ->components.at("0")
                  ->text_color,
              static_cast<int>(ViewColor::red));
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss)->components.at("0")->bg_color,
        static_cast<int>(ViewColor::green));
    EXPECT_EQ(
        data_->view_store.getRecv("a"_ss, "b"_ss)->components.at("1")->type,
        static_cast<int>(ViewComponentType::new_line));
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
        EXPECT_EQ(obj.field.u8String(), "a");
        EXPECT_EQ(obj.return_type, ValType::int_);
        EXPECT_EQ(obj.args.size(), 1u);
        EXPECT_EQ(obj.args.at(0)->name_, "a"_ss);
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
        EXPECT_EQ(obj.caller_id, 0u);
        EXPECT_EQ(obj.target_member_id, 10u);
        EXPECT_EQ(obj.field.u8String(), "b");
        EXPECT_EQ(obj.args.size(), 3u);
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
        EXPECT_EQ(obj.caller_id, 1u);
        EXPECT_EQ(obj.target_member_id, 10u);
        EXPECT_EQ(obj.field.u8String(), "b");
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
        EXPECT_EQ(obj.caller_id, 2u);
        EXPECT_EQ(obj.target_member_id, 10u);
        EXPECT_EQ(obj.field.u8String(), "b");
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
        EXPECT_EQ(obj.caller_id, 7u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.started, false);
    });
    dummy_s->recvClear();

    // arg error
    dummy_s->send(
        message::Call{8, 100, 0, "a"_ss, {ValAdaptor(1), ValAdaptor("zzz")}});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 8u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 8u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.is_error, true);
        EXPECT_FALSE(obj.result.empty());
    });
    dummy_s->recvClear();

    // throw
    dummy_s->send(message::Call{9, 100, 0, "a"_ss, {ValAdaptor(0)}});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 9u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 9u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.is_error, true);
        // 関数の中でthrowされた内容
        EXPECT_EQ(static_cast<std::string>(obj.result), "a==0");
    });
    dummy_s->recvClear();

    // success
    dummy_s->send(message::Call{19, 100, 0, "a"_ss, {ValAdaptor(123)}});
    wcli_->loopSyncFor(std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
    dummy_s->waitRecv<message::CallResponse>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 19u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.started, true);
    });
    dummy_s->waitRecv<message::CallResult>([&](const auto &obj) {
        EXPECT_EQ(obj.caller_id, 19u);
        EXPECT_EQ(obj.caller_member_id, 100u);
        EXPECT_EQ(obj.is_error, false);
        // 関数の中でthrowされた内容
        EXPECT_EQ(static_cast<int>(obj.result), 123);
    });
    dummy_s->recvClear();
}
