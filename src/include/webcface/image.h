#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/image.h"
#include "field.h"
#include "event_target.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
}
class Member;

class Image;
extern template class WEBCFACE_IMPORT EventTarget<Image>;

/*!
 * \brief (ver1.3から追加) 画像の送受信データを表すクラス
 *
 * コンストラクタではなく Member::image() を使って取得してください
 */
class WEBCFACE_DLL Image : protected Field, public EventTarget<Image> {
    std::optional<Common::ImageFrame> img = std::nullopt;

    void onAppend() const override final;

    Image &request(std::optional<int> rows, std::optional<int> cols,
                   Common::ImageCompressMode cmp_mode, int quality,
                   std::optional<Common::ImageColorMode> color_mode,
                   std::optional<double> frame_rate);

  public:
    Image() = default;
    Image(const Field &base);
    Image(const Field &base, std::u8string_view field)
        : Image(Field{base, field}) {}

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Image child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver1.12
     */
    Image child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Image child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Image operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.12
     */
    Image operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Image operator[](const char *field) const { return child(field); }
    /*!
     * \since ver1.12
     */
    Image operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Image operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Image parent() const { return this->Field::parent(); }

    /*!
     * \brief 画像をセットする
     *
     */
    Image &set(const ImageFrame &img);
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
    std::optional<ImageFrame> tryGet();
    /*!
     * \brief 画像を返す (データがない場合0x0の画像が返る)
     *
     * リクエストしていない場合すべてデフォルトで(元画像のフォーマットで)リクエストする
     *
     */
    ImageFrame get() { return tryGet().value_or(ImageFrame{}); }

    operator ImageFrame() { return get(); }

#if WEBCFACE_USE_OPENCV
    cv::Mat mat() &;
#endif
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     *
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    Image &free();

    //! 画像をクリア (リクエスト状態は解除しない)
    Image &clear();

    /*!
     * \brief Imageの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, Image>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};

WEBCFACE_NS_END
