#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/image.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;

class ImageFrameTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dp = std::make_shared<std::vector<unsigned char>>(100 * 100 * 3);
    }
    std::shared_ptr<std::vector<unsigned char>> dp;
};
TEST_F(ImageFrameTest, baseDefaultCtor) {
    ImageBase img;
    EXPECT_TRUE(img.empty());
    EXPECT_EQ(img.rows(), 0);
    EXPECT_EQ(img.cols(), 0);
    ASSERT_NE(img.dataPtr(), nullptr);
    EXPECT_EQ(img.dataPtr()->size(), 0);
    EXPECT_EQ(img.channels(), 1);
    EXPECT_EQ(img.color_mode(), ImageColorMode::gray);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
    EXPECT_EQ(img.data().size(), 0);
}
TEST_F(ImageFrameTest, baseRawPtrCtor) {
    ImageBase img(100, 100, dp->data());
    EXPECT_FALSE(img.empty());
    EXPECT_EQ(img.rows(), 100);
    EXPECT_EQ(img.cols(), 100);
    ASSERT_NE(img.dataPtr(), nullptr);
    EXPECT_NE(img.dataPtr()->data(), dp->data());
    EXPECT_EQ(img.dataPtr()->size(), 100*100*3);
    EXPECT_EQ(img.channels(), 3);
    EXPECT_EQ(img.color_mode(), ImageColorMode::bgr);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
}
TEST_F(ImageFrameTest, baseCopyCtor) {
    ImageBase img(100, 100, dp);
    ImageBase img2 = img;
    EXPECT_FALSE(img2.empty());
    EXPECT_EQ(img2.rows(), 100);
    EXPECT_EQ(img2.cols(), 100);
    ASSERT_NE(img2.dataPtr(), nullptr);
    EXPECT_EQ(img2.dataPtr(), img.dataPtr());
    EXPECT_EQ(img2.dataPtr(), dp);
    EXPECT_EQ(img2.channels(), 3);
    EXPECT_EQ(img2.color_mode(), ImageColorMode::bgr);
    EXPECT_EQ(img2.compress_mode(), ImageCompressMode::raw);
}
#if WEBCFACE_USE_OPENCV
TEST_F(ImageFrameTest, cvDefaultCtor) {
    ImageWithCV img;
    EXPECT_TRUE(img.empty());
    EXPECT_EQ(img.rows(), 0);
    EXPECT_EQ(img.cols(), 0);
    ASSERT_NE(img.dataPtr(), nullptr);
    EXPECT_EQ(img.dataPtr()->size(), 0);
    EXPECT_EQ(img.channels(), 1);
    EXPECT_EQ(img.color_mode(), ImageColorMode::gray);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
    EXPECT_EQ(img.data().size(), 0);
    EXPECT_TRUE(img.mat().empty());
}
TEST_F(ImageFrameTest, cvRawPtrCtor) {
    ImageWithCV img(100, 100, dp->data());
    EXPECT_FALSE(img.empty());
    EXPECT_EQ(img.rows(), 100);
    EXPECT_EQ(img.cols(), 100);
    ASSERT_NE(img.dataPtr(), nullptr);
    EXPECT_NE(img.dataPtr()->data(), dp->data());
    EXPECT_EQ(img.dataPtr()->size(), 100 * 100 * 3);
    EXPECT_EQ(img.channels(), 3);
    EXPECT_EQ(img.color_mode(), ImageColorMode::bgr);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
    EXPECT_FALSE(img.mat().empty());
    EXPECT_EQ(img.mat().rows, 100);
    EXPECT_EQ(img.mat().cols, 100);
    EXPECT_EQ(img.mat().channels(), 3);
}
TEST_F(ImageFrameTest, cvCopyCtor) {
    ImageBase img(100, 100, dp);
    ImageWithCV img2 = img;
    EXPECT_FALSE(img2.empty());
    EXPECT_EQ(img2.rows(), 100);
    EXPECT_EQ(img2.cols(), 100);
    ASSERT_NE(img2.dataPtr(), nullptr);
    EXPECT_EQ(img2.dataPtr(), img.dataPtr());
    EXPECT_EQ(img2.dataPtr(), dp);
    EXPECT_EQ(img2.channels(), 3);
    EXPECT_EQ(img2.color_mode(), ImageColorMode::bgr);
    EXPECT_EQ(img2.compress_mode(), ImageCompressMode::raw);
    EXPECT_EQ(img2.mat().rows, 100);
    EXPECT_EQ(img2.mat().cols, 100);
    EXPECT_EQ(img2.mat().channels(), 3);
}

#endif

class ImageTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    Image image(const std::string &member, const std::string &field) {
        return Image{Field{data_, member, field}};
    }
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(ImageTest, field) {
    EXPECT_EQ(image("a", "b").member().name(), "a");
    EXPECT_EQ(image("a", "b").name(), "b");
    EXPECT_EQ(image("a", "b").child("c").name(), "b.c");

    EXPECT_THROW(Image().tryGet(), std::runtime_error);
}
TEST_F(ImageTest, eventTarget) {
    image("a", "b").appendListener(callback<Image>());
    data_->image_change_event.dispatch(FieldBase{"a", "b"},
                                       Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ImageTest, imageSet) {
    data_->image_change_event.appendListener(FieldBase{self_name, "b"},
                                             callback());
    auto dp = std::make_shared<std::vector<unsigned char>>(100 * 100 * 3);
    image(self_name, "b").set(ImageFrame{100, 100, dp});
    EXPECT_EQ(data_->image_store.getRecv(self_name, "b")->rows() , 100);
    EXPECT_EQ(data_->image_store.getRecv(self_name, "b")->cols() , 100);
    EXPECT_EQ(data_->image_store.getRecv(self_name, "b")->dataPtr(), dp);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(image("a", "b").set(ImageFrame{}), std::invalid_argument);
}
TEST_F(ImageTest, imageRequest) {
    image("a", "1").request();
    EXPECT_EQ(data_->image_store.getReqInfo("a", "1"),
              (ImageReq{std::nullopt, std::nullopt, std::nullopt,
                        ImageCompressMode::raw, 0, std::nullopt}));
    image("a", "1").request(100, 100, ImageColorMode::rgba, 12.3);
    EXPECT_EQ(data_->image_store.getReqInfo("a", "1"),
              (ImageReq{100, 100, ImageColorMode::rgba, ImageCompressMode::raw,
                        0, 12.3}));
    image("a", "1").request(100, 100, ImageCompressMode::png, 9, 12.3);
    EXPECT_EQ(
        data_->image_store.getReqInfo("a", "1"),
        (ImageReq{100, 100, std::nullopt, ImageCompressMode::png, 9, 12.3}));
}
TEST_F(ImageTest, imageGet) {
    auto dp = std::make_shared<std::vector<unsigned char>>(100 * 100 * 3);
    data_->image_store.setRecv("a", "b", ImageFrame{100, 100, dp});
    EXPECT_EQ(image("a", "b").tryGet()->dataPtr(), dp);
    EXPECT_EQ(image("a", "b").get().dataPtr(), dp);
    EXPECT_EQ(image("a", "c").tryGet(), std::nullopt);
    EXPECT_TRUE(image("a", "c").get().empty());
    EXPECT_EQ(data_->image_store.transferReq().at("a").at("b"), 1);
    EXPECT_EQ(data_->image_store.getReqInfo("a", "b"),
              (ImageReq{std::nullopt, std::nullopt, std::nullopt,
                        ImageCompressMode::raw, 0, std::nullopt}));
    EXPECT_EQ(data_->image_store.transferReq().at("a").at("c"), 2);
    EXPECT_EQ(data_->image_store.getReqInfo("a", "c"),
              (ImageReq{std::nullopt, std::nullopt, std::nullopt,
                        ImageCompressMode::raw, 0, std::nullopt}));
    EXPECT_EQ(image(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->image_store.transferReq().count(self_name), 0);
    image("a", "d").appendListener(callback<Image>());
    EXPECT_EQ(data_->image_store.transferReq().at("a").at("d"), 3);
    EXPECT_EQ(data_->image_store.getReqInfo("a", "d"),
              (ImageReq{std::nullopt, std::nullopt, std::nullopt,
                        ImageCompressMode::raw, 0, std::nullopt}));
}
// todo: hidden, free
