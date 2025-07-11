#include "test_common.h"
#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/image.h>
#include <stdexcept>

using namespace webcface;

class ImageFrameTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dp = std::make_shared<std::vector<unsigned char>>(100 * 100 * 3);
    }
    std::shared_ptr<std::vector<unsigned char>> dp;
};

[[maybe_unused]] void imageEnumTest(ImageColorMode color,
                                    ImageCompressMode comp) {
    // 新しいcolorを追加してここに書き忘れたらwarningが出る
    switch (color) {
    case ImageColorMode::gray:
        static_assert(static_cast<int>(ImageColorMode::gray) ==
                      static_cast<int>(message::ImageColorMode::gray));
        break;
    case ImageColorMode::bgr:
        static_assert(static_cast<int>(ImageColorMode::bgr) ==
                      static_cast<int>(message::ImageColorMode::bgr));
        break;
    case ImageColorMode::bgra:
        static_assert(static_cast<int>(ImageColorMode::bgra) ==
                      static_cast<int>(message::ImageColorMode::bgra));
        break;
    case ImageColorMode::rgb:
        static_assert(static_cast<int>(ImageColorMode::rgb) ==
                      static_cast<int>(message::ImageColorMode::rgb));
        break;
    case ImageColorMode::rgba:
        static_assert(static_cast<int>(ImageColorMode::rgba) ==
                      static_cast<int>(message::ImageColorMode::rgba));
        break;
    }
    switch (comp) {
    case ImageCompressMode::raw:
        static_assert(static_cast<int>(ImageCompressMode::raw) ==
                      static_cast<int>(message::ImageCompressMode::raw));
        break;
    case ImageCompressMode::jpeg:
        static_assert(static_cast<int>(ImageCompressMode::jpeg) ==
                      static_cast<int>(message::ImageCompressMode::jpeg));
        break;
    case ImageCompressMode::webp:
        static_assert(static_cast<int>(ImageCompressMode::webp) ==
                      static_cast<int>(message::ImageCompressMode::webp));
        break;
    case ImageCompressMode::png:
        static_assert(static_cast<int>(ImageCompressMode::png) ==
                      static_cast<int>(message::ImageCompressMode::png));
        break;
    }
}

TEST_F(ImageFrameTest, baseDefaultCtor) {
    ImageFrame img;
    EXPECT_TRUE(img.empty());
    EXPECT_EQ(img.rows(), 0);
    EXPECT_EQ(img.cols(), 0);
    ASSERT_NE(img.dataPtr(), nullptr);
    EXPECT_EQ(img.dataPtr()->size(), 0u);
    EXPECT_EQ(img.channels(), 1);
    EXPECT_EQ(img.color_mode(), ImageColorMode::gray);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
    EXPECT_EQ(img.data().size(), 0u);
}
TEST_F(ImageFrameTest, copyCtor) {
    ImageFrame img2(sizeHW(100, 100), dp->data(), ImageColorMode::bgr);
    ImageFrame img = img2; // NOLINT
    EXPECT_FALSE(img.empty());
    EXPECT_EQ(img.rows(), img2.rows());
    EXPECT_EQ(img.cols(), img2.cols());
    ASSERT_NE(img.dataPtr(), nullptr);
    ASSERT_NE(img2.dataPtr(), nullptr);
    ASSERT_EQ(img.dataPtr(), img2.dataPtr());
    EXPECT_EQ(img.dataPtr()->size(), 100u * 100u * 3u);
    EXPECT_EQ(img.channels(), img2.channels());
    EXPECT_EQ(img.color_mode(), img2.color_mode());
    EXPECT_EQ(img.compress_mode(), img2.compress_mode());
}
TEST_F(ImageFrameTest, moveCtor) {
    ImageFrame img2(sizeHW(100, 100), dp->data(), ImageColorMode::bgr);
    ImageFrame img = std::move(img2);
    EXPECT_FALSE(img.empty());
    EXPECT_TRUE(img2.empty()); // NOLINT
    EXPECT_EQ(img.rows(), 100);
    EXPECT_EQ(img.cols(), 100);
    ASSERT_NE(img.dataPtr(), nullptr);
    ASSERT_NE(img2.dataPtr(), nullptr); // NOLINT
    EXPECT_EQ(img.dataPtr()->size(), 100u * 100u * 3u);
    EXPECT_EQ(img.channels(), 3);
    EXPECT_EQ(img.color_mode(), ImageColorMode::bgr);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
}
TEST_F(ImageFrameTest, baseRawPtrCtor) {
    ImageFrame img(sizeHW(100, 100), dp->data(), ImageColorMode::bgr);
    EXPECT_FALSE(img.empty());
    EXPECT_EQ(img.rows(), 100);
    EXPECT_EQ(img.cols(), 100);
    ASSERT_NE(img.dataPtr(), nullptr);
    EXPECT_NE(img.dataPtr()->data(), dp->data());
    EXPECT_EQ(img.dataPtr()->size(), 100u * 100u * 3u);
    EXPECT_EQ(img.channels(), 3);
    EXPECT_EQ(img.color_mode(), ImageColorMode::bgr);
    EXPECT_EQ(img.compress_mode(), ImageCompressMode::raw);
}
TEST_F(ImageFrameTest, baseCopyCtor) {
    ImageFrame img(sizeHW(100, 100), dp, ImageColorMode::bgr);
    ImageFrame img2 = img; // NOLINT
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

class ImageTest : public ::testing::Test {
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
    Image image(const T1 &member, const T2 &name) {
        return Image{field(member, name)};
    }

    int callback_called;
    template <typename V = Image>
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
    image("a", "b").onChange(callback<Image>());
    data_->image_change_event.lock().get()["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ImageTest, imageSet) {
    data_->image_change_event.lock().get()[self_name]["b"_ss] =
        std::make_shared<std::function<void(Image)>>(callback());
    auto dp = std::make_shared<std::vector<unsigned char>>(100 * 100 * 3);
    image(self_name, "b")
        .set(ImageFrame{sizeHW(100, 100), dp, ImageColorMode::bgr});
    EXPECT_EQ(data_->image_store.getRecv(self_name, "b"_ss)->rows(), 100);
    EXPECT_EQ(data_->image_store.getRecv(self_name, "b"_ss)->cols(), 100);
    EXPECT_EQ(data_->image_store.getRecv(self_name, "b"_ss)->dataPtr(), dp);
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(image("a", "b").set(ImageFrame{}), std::invalid_argument);
}
TEST_F(ImageTest, imageRequest) {
    image("a", "1").request();
    EXPECT_EQ(
        data_->image_store.getReqInfo("a"_ss, "1"_ss),
        (message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                           message::ImageCompressMode::raw, 0, std::nullopt}));
    image("a", "1").request(sizeHW(100, 100), ImageColorMode::rgba, 12.3);
    EXPECT_EQ(data_->image_store.getReqInfo("a"_ss, "1"_ss),
              (message::ImageReq{100, 100, message::ImageColorMode::rgba,
                                 message::ImageCompressMode::raw, 0, 12.3}));
    image("a", "1").request(sizeHW(100, 100), ImageCompressMode::png, 9, 12.3);
    EXPECT_EQ(data_->image_store.getReqInfo("a"_ss, "1"_ss),
              (message::ImageReq{100, 100, std::nullopt,
                                 message::ImageCompressMode::png, 9, 12.3}));
}
TEST_F(ImageTest, imageGet) {
    auto dp = std::make_shared<std::vector<unsigned char>>(100 * 100 * 3);
    data_->image_store.setRecv(
        "a"_ss, "b"_ss, ImageFrame{sizeHW(100, 100), dp, ImageColorMode::bgr});
    ASSERT_TRUE(image("a", "b").tryGet().has_value());
    EXPECT_EQ(image("a", "b").tryGet()->dataPtr(), dp);
    EXPECT_EQ(image("a", "b").get().dataPtr(), dp);
    EXPECT_EQ(image("a", "c").tryGet(), std::nullopt);
    EXPECT_TRUE(image("a", "c").get().empty());
    EXPECT_EQ(data_->image_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(
        data_->image_store.getReqInfo("a"_ss, "b"_ss),
        (message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                           message::ImageCompressMode::raw, 0, std::nullopt}));
    EXPECT_EQ(data_->image_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(
        data_->image_store.getReqInfo("a"_ss, "c"_ss),
        (message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                           message::ImageCompressMode::raw, 0, std::nullopt}));
    EXPECT_EQ(image(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->image_store.transferReq().count(self_name), 0u);
    image("a", "d").onChange(callback<Image>());
    EXPECT_EQ(data_->image_store.transferReq().at("a"_ss).at("d"_ss), 3u);
    EXPECT_EQ(
        data_->image_store.getReqInfo("a"_ss, "d"_ss),
        (message::ImageReq{std::nullopt, std::nullopt, std::nullopt,
                           message::ImageCompressMode::raw, 0, std::nullopt}));
}
// todo: hidden, free
