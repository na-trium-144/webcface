#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/robot_model.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
class RobotModelTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::u8string self_name = u8"test";
    std::shared_ptr<Internal::ClientData> data_;
    FieldBase fieldBase(std::u8string_view member,
                        std::string_view name) const {
        return FieldBase{member, Encoding::castToU8(name)};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{Encoding::castToU8(member), Encoding::castToU8(name)};
    }
    Field field(std::u8string_view member, std::string_view name = "") const {
        return Field{data_, member, Encoding::castToU8(name)};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, Encoding::castToU8(member),
                     Encoding::castToU8(name)};
    }
    template <typename T1, typename T2>
    RobotModel model(const T1 &member, const T2 &name) {
        return RobotModel{field(member, name)};
    }
    int callback_called;
    template <typename V = RobotModel>
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
    data_->robot_model_change_event[fieldBase("a", "b")]->operator()(
        field("a", "b"));
    EXPECT_EQ(callback_called, 1);
}
TEST_F(RobotModelTest, set) {
    data_->robot_model_change_event[fieldBase(self_name, "b")]->append(
        callback());
    model(self_name, "b").set({RobotLink{"a", Geometry{}, ViewColor::black}});
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, u8"b"))->size(), 1);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(model("a", "b").set({}), std::invalid_argument);
}
TEST_F(RobotModelTest, sync) {
    data_->robot_model_change_event[fieldBase(self_name, "b")]->append(
        callback());
    auto m = model(self_name, "b");
    m << RobotLink{"1", Geometry{}, ViewColor::black};
    m << RobotLink{"2", Geometry{}, ViewColor::black};
    m << RobotLink{"3", Geometry{}, ViewColor::black};
    m.sync();
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, u8"b"))->size(), 3);
    EXPECT_EQ(callback_called, 1);

    auto m3 = model(self_name, "b");
    m3.init();
    m3.sync();
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, u8"b"))->size(), 0);

    {
        auto m2 = model(self_name, "b2");
        m2 << RobotLink{"1", Geometry{}, ViewColor::black};
        m2 << RobotLink{"2", Geometry{}, ViewColor::black};
        m2 << RobotLink{"3", Geometry{}, ViewColor::black};
    }
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, u8"b2"))->size(),
              3);

    EXPECT_THROW(model("a", "b").init().sync(), std::invalid_argument);
}

TEST_F(RobotModelTest, get) {
    data_->robot_model_store.setRecv(
        u8"a", u8"b",
        std::make_shared<std::vector<RobotLink>>(
            std::vector<RobotLink>{{"a", Geometry{}, ViewColor::black}}));
    EXPECT_EQ(model("a", "b").tryGet()->size(), 1);
    EXPECT_EQ(model("a", "b").get().size(), 1);
    EXPECT_EQ(model("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(model("a", "c").get().size(), 0);
    EXPECT_EQ(data_->robot_model_store.transferReq().at(u8"a").at(u8"b"), 1);
    EXPECT_EQ(data_->robot_model_store.transferReq().at(u8"a").at(u8"c"), 2);
    EXPECT_EQ(model(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->robot_model_store.transferReq().count(self_name), 0);
    model("a", "d").appendListener(callback<RobotModel>());
    EXPECT_EQ(data_->robot_model_store.transferReq().at(u8"a").at(u8"d"), 3);
}
