#pragma once
#include <optional>
#include <vector>
#include <memory>
#include <cstddef>
#include <stdexcept>
#include <concepts>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Message {
struct ImageFrame;
} // namespace Message

enum class ImageColorMode {
    gray = 0,
    bgr = 1,
    bgra = 2,
    rgb = 3,
    rgba = 4,
};
enum class ImageCompressMode {
    raw = 0,
    jpeg = 1,
    webp = 2,
    png = 3,
};

class Size {
    std::size_t w_, h_;
    Size(std::size_t width, std::size_t height) : w_(width), h_(height) {}

  public:
    Size() = default;
    friend Size sizeWH(std::size_t width, std::size_t height);
    friend Size sizeHW(std::size_t height, std::size_t width);
    std::size_t width() const { return w_; }
    std::size_t height() const { return h_; }
    std::size_t rows() const { return h_; }
    std::size_t cols() const { return w_; }
};

/*!
 * \brief 幅 × 高さ でサイズを指定
 * \since ver2.0
 */
inline Size sizeWH(std::size_t width, std::size_t height) {
    return Size{width, height};
}
/*!
 * \brief 高さ × 幅 でサイズを指定
 * \since ver2.0
 */
inline Size sizeHW(std::size_t height, std::size_t width) {
    return Size{width, height};
}

class SizeOption {
    std::optional<std::size_t> w_, h_;
    SizeOption(std::optional<std::size_t> width,
               std::optional<std::size_t> height)
        : w_(width), h_(height) {}

  public:
    SizeOption() = default;
    SizeOption(const Size &s) : w_(s.width()), h_(s.height()) {}
    template <typename T1, typename T2>
    friend Size sizeWH(T1 width, T2 height);
    template <typename T1, typename T2>
    friend Size sizeHW(T1 height, T2 width);
    std::optional<std::size_t> rows() const { return h_; }
    std::optional<std::size_t> cols() const { return w_; }
};

/*!
 * \brief 幅 × 高さ でサイズを指定
 * \since ver2.0
 */
template <typename T1, typename T2>
    requires(std::same_as<T1, std::nullopt_t> ||
             std::same_as<T2, std::nullopt_t>)
inline SizeOption sizeWH(T1 width, T2 height) {
    return SizeOption{width, height};
}
/*!
 * \brief 高さ × 幅 でサイズを指定
 * \since ver2.0
 */
template <typename T1, typename T2>
    requires(std::same_as<T1, std::nullopt_t> ||
             std::same_as<T2, std::nullopt_t>)
inline SizeOption sizeHW(T1 height, T2 width) {
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
    ImageFrame(const Message::ImageFrame &m);
    Message::ImageFrame toMessage() const;
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
        : ImageFrame(sizeHW(static_cast<std::size_t>(rows),
                            static_cast<std::size_t>(cols)),
                     data, color_mode) {}
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
    std::size_t width() const { return size_.width(); }
    /*!
     * \brief 画像の高さ
     * \since ver2.0
     */
    std::size_t height() const { return size_.height(); }
    /*!
     * \brief 画像の高さ
     *
     */
    std::size_t rows() const { return size_.rows(); }
    /*!
     * \brief 画像の幅
     *
     */
    std::size_t cols() const { return size_.cols(); }
    /*!
     * \brief 1ピクセル当たりのデータサイズ(byte数)を取得
     *
     * \return 1, 3, or 4
     *
     */
    std::size_t channels() const;
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
    const unsigned char &at(std::size_t row, std::size_t col,
                            std::size_t ch = 0) const {
        return dataPtr()->at((row * cols() + col) * channels() + ch);
    }
    /*!
     * \brief 画像の要素にアクセス
     * \since ver2.0
     *
     * compress_modeがrawでない場合は正常にアクセスできない。
     *
     */
    unsigned char &at(std::size_t row, std::size_t col, std::size_t ch = 0) {
        return dataPtr()->at((row * cols() + col) * channels() + ch);
    }
};

using ImageBase [[deprecated]] = ImageFrame;

// struct WEBCFACE_DLL ImageReq {
//     std::optional<int> rows = std::nullopt, cols = std::nullopt;
//     std::optional<ImageColorMode> color_mode = std::nullopt;
//     ImageCompressMode cmp_mode = ImageCompressMode::raw;
//     int quality = 0;
//     std::optional<double> frame_rate = std::nullopt;

//     ImageReq() = default;
//     ImageReq(std::optional<int> rows, std::optional<int> cols,
//              std::optional<ImageColorMode> color_mode,
//              ImageCompressMode cmp_mode, int quality,
//              std::optional<double> frame_rate)
//         : rows(rows), cols(cols), color_mode(color_mode), cmp_mode(cmp_mode),
//           quality(quality), frame_rate(frame_rate) {}
//     ImageReq(const Message::ImageReq &m);
//     Message::ImageReq toMessage() const;
    
//     bool operator==(const ImageReq &rhs) const;
// };

WEBCFACE_NS_END
