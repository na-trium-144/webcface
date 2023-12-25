#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/image.h"
#include "field.h"
#include "event_target.h"
#include "common/def.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
class Member;

//! 画像の送受信データを表すクラス
/*! コンストラクタではなく Member::image() を使って取得してください
 */
class Image : protected Field, public EventTarget<Image> {
    std::optional<Common::ImageFrame> img = std::nullopt;

    WEBCFACE_DLL void onAppend() const override;

  public:
    Image() = default;
    WEBCFACE_DLL Image(const Field &base);
    WEBCFACE_DLL Image(const Field &base, const std::string &field)
        : Image(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 子フィールドを返す
    /*!
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするValue
     */
    Image child(const std::string &field) {
        return Image{*this, this->field_ + "." + field};
    }

    //! 画像をセットする
    WEBCFACE_DLL Image &set(const ImageFrame &img);
    //! 画像をセットする
    Image &operator=(const ImageFrame &img) {
        this->set(img);
        return *this;
    }

    Image &
    request(std::optional<int> rows = std::nullopt,
            std::optional<int> cols = std::nullopt,
            std::optional<Common::ImageColorMode> color_mode = std::nullopt,
            std::optional<double> frame_rate = std::nullopt) {
        return request(rows, cols, Common::ImageCompressMode::raw, 0,
                       color_mode, frame_rate);
    }
    Image &request(std::optional<int> rows, std::optional<int> cols,
                   Common::ImageCompressMode cmp_mode, int quality,
                   std::optional<double> frame_rate = std::nullopt) {
        return request(rows, cols, cmp_mode, quality, std::nullopt, frame_rate);
    }
    WEBCFACE_DLL Image &
    request(std::optional<int> rows, std::optional<int> cols,
            Common::ImageCompressMode cmp_mode, int quality,
            std::optional<Common::ImageColorMode> color_mode,
            std::optional<double> frame_rate);
    //! 画像を返す
    WEBCFACE_DLL std::optional<ImageFrame> tryGet() const;
    //! 画像を返す (データがない場合0x0の画像が返る)
    ImageFrame get() const { return tryGet().value_or(ImageFrame{}); }

    operator ImageFrame() const { return get(); }

#if WEBCFACE_USE_OPENCV
    WEBCFACE_DLL cv::Mat mat() &;
#endif
    //! syncの時刻を返す
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL Image &free();

    //! 画像をクリア (リクエスト状態は解除しない)
    WEBCFACE_DLL Image &clear();
};

} // namespace WEBCFACE_NS
