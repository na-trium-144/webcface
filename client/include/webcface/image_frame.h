#pragma once
#include <optional>
#include <vector>
#include <memory>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/encoding/image_mode.h"

WEBCFACE_NS_BEGIN
namespace message {
struct ImageFrame;
} // namespace message

class Size {
    int w_, h_;
    Size(int width, int height) : w_(width), h_(height) {}

  public:
    Size() = default;
    friend Size WEBCFACE_CALL sizeWH(int width, int height);
    friend Size WEBCFACE_CALL sizeHW(int height, int width);
    int width() const { return w_; }
    int height() const { return h_; }
    int rows() const { return h_; }
    int cols() const { return w_; }
};

/*!
 * \brief 幅 × 高さ でサイズを指定
 * \since ver2.0
 */
inline Size WEBCFACE_CALL sizeWH(int width, int height) {
    return Size{width, height};
}
/*!
 * \brief 高さ × 幅 でサイズを指定
 * \since ver2.0
 */
inline Size WEBCFACE_CALL sizeHW(int height, int width) {
    return Size{width, height};
}

class SizeOption {
    std::optional<int> w_, h_;
    SizeOption(std::optional<int> width, std::optional<int> height)
        : w_(width), h_(height) {}

  public:
    SizeOption() = default;
    SizeOption(const Size &s) : w_(s.width()), h_(s.height()) {}
    friend SizeOption WEBCFACE_CALL sizeWH(std::optional<int> width,
                                           std::optional<int> height);
    friend SizeOption WEBCFACE_CALL sizeHW(std::optional<int> height,
                                           std::optional<int> width);
    std::optional<int> rows() const { return h_; }
    std::optional<int> cols() const { return w_; }
};

/*!
 * \brief 幅 × 高さ でサイズを指定
 * \since ver2.0
 */
inline SizeOption WEBCFACE_CALL sizeWH(std::optional<int> width,
                                       std::optional<int> height) {
    return SizeOption{width, height};
}
/*!
 * \brief 高さ × 幅 でサイズを指定
 * \since ver2.0
 */
inline SizeOption WEBCFACE_CALL sizeHW(std::optional<int> height,
                                       std::optional<int> width) {
    return SizeOption{width, height};
}

/*!
 * \brief (ver1.3から追加) 画像データ
 *
 * * 8bitのグレースケール, BGR, BGRAフォーマットのみを扱う
 * * 画像受信時にはjpegやpngなどにエンコードされたデータが入ることもある
 * * データはshared_ptrで保持され、Imageをコピーしてもコピーされない
 *
 */
class WEBCFACE_DLL ImageFrame {
  protected:
    Size size_;
    std::shared_ptr<std::vector<unsigned char>> data_;
    ImageColorMode color_mode_;
    ImageCompressMode cmp_mode_;

  public:
    /*!
     * \brief 空の(0x0の) ImageFrameを作成
     *
     */
    ImageFrame();
    ImageFrame(const Size &size,
               const std::shared_ptr<std::vector<unsigned char>> &data,
               ImageColorMode color_mode = ImageColorMode::bgr,
               ImageCompressMode cmp_mode = ImageCompressMode::raw);
    ImageFrame(const message::ImageFrame &m);
    message::ImageFrame toMessage() const;
    /*!
     * \brief 生画像データの配列からImageFrameを作成
     *
     * dataから rows * cols * channels バイトがコピーされる
     *
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * \param data 画像データ
     * \param color_mode データの構造を指定
     * (デフォルトはOpenCVのBGR, uint8*3バイト)
     * \deprecated ver2.0〜 rows, colsの順番がややこしいので sizeHW()
     * を使ってサイズ指定
     *
     */
    [[deprecated("Ambiguous image size")]] ImageFrame(
        int rows, int cols, const void *data,
        ImageColorMode color_mode = ImageColorMode::bgr)
        : ImageFrame(sizeHW(rows, cols), data, color_mode) {}
    /*!
     * \brief 生画像データの配列からImageFrameを作成
     * \since ver2.0
     *
     * dataから width * height * channels バイトがコピーされる
     *
     * \param size 画像のサイズ (sizeHW または sizeWH)
     * \param cols 画像の幅
     * \param data 画像データ
     * \param color_mode データの構造を指定
     *
     */
    ImageFrame(const Size &size, const void *data, ImageColorMode color_mode);
    /*!
     * \brief 空のImageFrameを作成
     * \since ver2.0
     *
     * width * height * channels バイトのバッファが生成されるので、
     * 作成後にdata()またはat()でデータを書き込んで使う
     *
     * \param size 画像のサイズ (sizeHW または sizeWH)
     * \param cols 画像の幅
     * \param color_mode データの構造を指定
     *
     */
    ImageFrame(const Size &size, ImageColorMode color_mode);

    /*!
     * \brief 画像が空かどうかを返す
     *
     * \return dataPtr()->size() == 0
     *
     */
    bool empty() const { return data_->size() == 0; }
    /*!
     * \brief 画像のサイズ
     * \since ver2.0
     */
    const Size &size() const { return size_; }
    /*!
     * \brief 画像の幅
     * \since ver2.0
     */
    int width() const { return size_.width(); }
    /*!
     * \brief 画像の高さ
     * \since ver2.0
     */
    int height() const { return size_.height(); }
    /*!
     * \brief 画像の高さ
     *
     */
    int rows() const { return size_.rows(); }
    /*!
     * \brief 画像の幅
     *
     */
    int cols() const { return size_.cols(); }
    /*!
     * \brief 1ピクセル当たりのデータサイズ(byte数)を取得
     *
     * \return 1, 3, or 4
     *
     */
    int channels() const;
    /*!
     * \sa colorMode()
     */
    ImageColorMode color_mode() const { return color_mode_; }
    /*!
     * \brief 色の並び順 (生画像データの場合)
     *
     * compressModeがrawでない場合意味を持たない。
     *
     */
    ImageColorMode colorMode() const { return color_mode_; }
    /*!
     * \sa compressMode()
     */
    ImageCompressMode compress_mode() const { return cmp_mode_; }
    /*!
     * \brief 画像の圧縮モード
     *
     */
    ImageCompressMode compressMode() const { return cmp_mode_; }

    std::shared_ptr<std::vector<unsigned char>> dataPtr() const {
        return data_;
    }
    /*!
     * \brief 画像データ
     *
     * \return compress_modeがrawの場合、rows * cols * channels
     * 要素の画像データ。 それ以外の場合、圧縮された画像のデータ
     * (ver2.0〜非const)
     *
     */
    const std::vector<unsigned char> &data() const { return *data_; }
    /*!
     * \brief 画像データ (非const)
     * \since ver2.0
     * \return compress_modeがrawの場合、rows * cols * channels
     * 要素の画像データ。 それ以外の場合、圧縮された画像のデータ
     *
     */
    std::vector<unsigned char> &data() { return *data_; }
    /*!
     * \brief 画像の要素にアクセス
     *
     * compress_modeがrawでない場合は正常にアクセスできない。
     *
     */
    const unsigned char &at(int row, int col, int ch = 0) const {
        return dataPtr()->at((row * cols() + col) * channels() + ch);
    }
    /*!
     * \brief 画像の要素にアクセス
     * \since ver2.0
     *
     * compress_modeがrawでない場合は正常にアクセスできない。
     *
     */
    unsigned char &at(int row, int col, int ch = 0) {
        return dataPtr()->at((row * cols() + col) * channels() + ch);
    }
};

using ImageBase [[deprecated]] = ImageFrame;

WEBCFACE_NS_END
