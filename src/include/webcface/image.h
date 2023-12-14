#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/image.h"
#include "field.h"
#include "event_target.h"

namespace webcface {
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

    Image &request(std::optional<int> rows = std::nullopt,
                   std::optional<int> cols = std::nullopt, int channels = 3) {
        return request(rows, cols, channels, Common::ImageCompressMode::raw, 0);
    }
    WEBCFACE_DLL Image &request(std::optional<int> rows,
                                std::optional<int> cols, int channels,
                                Common::ImageCompressMode mode, int quality);
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

} // namespace webcface
