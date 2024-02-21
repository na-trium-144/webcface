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

/*!
 * \brief (ver1.3から追加) 画像の送受信データを表すクラス
 *
 * コンストラクタではなく Member::image() を使って取得してください
 */
class Image : protected Field, public EventTarget<Image> {
    std::optional<Common::ImageFrame> img = std::nullopt;

    WEBCFACE_DLL void onAppend() const override;

    WEBCFACE_DLL Image &
    request(std::optional<int> rows, std::optional<int> cols,
            Common::ImageCompressMode cmp_mode, int quality,
            std::optional<Common::ImageColorMode> color_mode,
            std::optional<double> frame_rate);

  public:
    Image() = default;
    WEBCFACE_DLL Image(const Field &base);
    WEBCFACE_DLL Image(const Field &base, const std::string &field)
        : Image(Field{base, field}) {}

    using Field::member;
    using Field::name;

    /*!
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするImage
     *
     */
    Image child(const std::string &field) {
        return Image{*this, this->field_ + "." + field};
    }

    /*!
     * \brief 画像をセットする
     *
     */
    WEBCFACE_DLL Image &set(const ImageFrame &img);
    /*!
     * \brief 画像をセットする
     *
     */
    Image &operator=(const ImageFrame &img) {
        this->set(img);
        return *this;
    }

    /*!
     * \brief 画像を生画像のフォーマットでリクエストする
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * rows,colsのどちらかのみがnulloptの場合縦横比を保ってリサイズ
     * \param color_mode 画像の色フォーマット
     * (nulloptの場合元画像のフォーマット)
     * \param frame_rate 画像を受信する頻度
     * (指定しない場合元画像が更新されるたびに受信する)
     *
     */
    Image &
    request(std::optional<int> rows = std::nullopt,
            std::optional<int> cols = std::nullopt,
            std::optional<Common::ImageColorMode> color_mode = std::nullopt,
            std::optional<double> frame_rate = std::nullopt) {
        return request(rows, cols, Common::ImageCompressMode::raw, 0,
                       color_mode, frame_rate);
    }
    /*!
     * \brief 画像を圧縮されたフォーマットでリクエストする
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * rows,colsのどちらかのみがnulloptの場合縦横比を保ってリサイズ
     * \param cmp_mode 圧縮モード
     * \param quality 圧縮のパラメータ
     * * jpeg → 0〜100 (大きいほうが高品質)
     * * png → 0〜9 (大きいほうが圧縮後のサイズが小さい)
     * * webp → 1〜100 (大きいほうが高品質)
     * \param frame_rate 画像を受信する頻度
     * (指定しない場合元画像が更新されるたびに受信する)
     *
     */
    Image &request(std::optional<int> rows, std::optional<int> cols,
                   Common::ImageCompressMode cmp_mode, int quality,
                   std::optional<double> frame_rate = std::nullopt) {
        return request(rows, cols, cmp_mode, quality, std::nullopt, frame_rate);
    }
    /*!
     * \brief 画像を返す
     *
     * リクエストしていない場合すべてデフォルトで(元画像のフォーマットで)リクエストする
     *
     */
    WEBCFACE_DLL std::optional<ImageFrame> tryGet();
    /*!
     * \brief 画像を返す (データがない場合0x0の画像が返る)
     *
     * リクエストしていない場合すべてデフォルトで(元画像のフォーマットで)リクエストする
     *
     */
    ImageFrame get() { return tryGet().value_or(ImageFrame{}); }

    operator ImageFrame() { return get(); }

#if WEBCFACE_USE_OPENCV
    WEBCFACE_DLL cv::Mat mat() &;
#endif
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     *
     */
    [[deprecated]] WEBCFACE_DLL std::chrono::system_clock::time_point
    time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL Image &free();

    //! 画像をクリア (リクエスト状態は解除しない)
    WEBCFACE_DLL Image &clear();
};

} // namespace WEBCFACE_NS
