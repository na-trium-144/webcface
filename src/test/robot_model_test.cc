#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/robot_model.h>
#include <stdexcept>
#include <chrono>

using namespace WEBCFACE_NS;
class RobotModelTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    RobotModel model(const std::string &member, const std::string &field) {
        return RobotModel{Field{data_, member, field}};
    }
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(RobotModelTest, field) {
    EXPECT_EQ(model("a", "b").member().name(), "a");
    EXPECT_EQ(model("a", "b").name(), "b");

    EXPECT_THROW(RobotModel().tryGet(), std::runtime_error);
}
TEST_F(RobotModelTest, eventTarget) {
    model("a", "b").appendListener(callback<RobotModel>());
    data_->robot_model_change_event.dispatch(FieldBase{"a", "b"},
                                             Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
}
TEST_F(RobotModelTest, set) {
    data_->robot_model_change_event.appendListener(FieldBase{self_name, "b"},
                                                   callback());
    model(self_name, "b").set({RobotLink{"a", Geometry{}, ViewColor::black}});
    EXPECT_EQ(data_->robot_model_store.getRecv(self_name, "b")->size(), 1);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(model("a", "b").set({}), std::invalid_argument);
}

TEST_F(RobotModelTest, get) {
    data_->robot_model_store.setRecv(
        "a", "b", {RobotLink{"a", Geometry{}, ViewColor::black}});
    EXPECT_EQ(model("a", "b").tryGet()->size(), 1);
    EXPECT_EQ(model("a", "b").get().size(), 1);
    EXPECT_EQ(model("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(model("a", "c").get().size(), 0);
    EXPECT_EQ(data_->robot_model_store.transferReq().at("a").at("b"), 1);
    EXPECT_EQ(data_->robot_model_store.transferReq().at("a").at("c"), 2);
    EXPECT_EQ(model(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->robot_model_store.transferReq().count(self_name), 0);
    model("a", "d").appendListener(callback<RobotModel>());
    EXPECT_EQ(data_->robot_model_store.transferReq().at("a").at("d"), 3);
}
TEST_F(RobotModelTest, time) {
    auto t = std::chrono::system_clock::now();
    data_->sync_time_store.setRecv("a", t);
    EXPECT_EQ(model("a", "b").time(), t);
}
