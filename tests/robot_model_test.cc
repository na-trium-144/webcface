#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/robot_model.h>
#include <stdexcept>
#include "webcface/internal/robot_link_internal.h"

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class RobotModelTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
        callback_called = 0;
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
    FieldBase fieldBase(const SharedString &member,
                        std::string_view name) const {
        return FieldBase{member, SharedString::fromU8String(name)};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{SharedString::fromU8String(member),
                         SharedString::fromU8String(name)};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString::fromU8String(name)};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, SharedString::fromU8String(member),
                     SharedString::fromU8String(name)};
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
    model("a", "b").onChange(callback<RobotModel>());
    data_->robot_model_change_event["a"_ss]["b"_ss]->operator()(
        field("a", "b"));
    EXPECT_EQ(callback_called, 1);
}
TEST_F(RobotModelTest, set) {
    data_->robot_model_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(RobotModel)>>(callback());
    model(self_name, "b").set({RobotLink{"a", Geometry{}, ViewColor::black}});
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, "b"_ss))->size(),
              1u);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(model("a", "b").set({}), std::invalid_argument);
}
TEST_F(RobotModelTest, sync) {
    data_->robot_model_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(RobotModel)>>(callback());
    auto m = model(self_name, "b");
    m << RobotLink{"1", Geometry{}, ViewColor::black};
    m << RobotLink{"2", Geometry{}, ViewColor::black};
    m << RobotLink{"3", Geometry{}, ViewColor::black};
    m.sync();
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, "b"_ss))->size(),
              3u);
    EXPECT_EQ(callback_called, 1);

    auto m3 = model(self_name, "b");
    m3.init();
    m3.sync();
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, "b"_ss))->size(),
              0u);

    {
        auto m2 = model(self_name, "b2");
        m2 << RobotLink{"1", Geometry{}, ViewColor::black};
        m2 << RobotLink{"2", Geometry{}, ViewColor::black};
        m2 << RobotLink{"3", Geometry{}, ViewColor::black};
    }
    EXPECT_EQ((*data_->robot_model_store.getRecv(self_name, "b2"_ss))->size(),
              3u);

    EXPECT_THROW(model("a", "b").init().sync(), std::invalid_argument);
}

TEST_F(RobotModelTest, get) {
    auto ln = std::make_shared<internal::RobotLinkData>();
    ln->name = SharedString::fromU8String("a");
    data_->robot_model_store.setRecv(
        "a"_ss, "b"_ss,
        std::make_shared<std::vector<std::shared_ptr<internal::RobotLinkData>>>(
            std::vector<std::shared_ptr<internal::RobotLinkData>>{ln}));
    EXPECT_EQ(model("a", "b").tryGet()->size(), 1u);
    EXPECT_EQ(model("a", "b").get().size(), 1u);
    EXPECT_EQ(model("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(model("a", "c").get().size(), 0u);
    EXPECT_EQ(data_->robot_model_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->robot_model_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(model(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->robot_model_store.transferReq().count(self_name), 0u);
    model("a", "d").onChange(callback<RobotModel>());
    EXPECT_EQ(data_->robot_model_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}
