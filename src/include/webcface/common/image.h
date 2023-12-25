#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include "def.h"

// todo: cmakeなしでヘッダー読んだときにopencvの有無を判別する
#if WEBCFACE_USE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

namespace WEBCFACE_NS {
inline namespace Common {

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

/*!
 * \brief 画像データ
 *
 * * 8bitのグレースケール, BGR, BGRAフォーマットのみを扱う
 * * 画像受信時にはjpegやpngなどにエンコードされたデータが入ることもある
 * * データはshared_ptrで保持され、Imageをコピーしてもコピーされない
 *
 */
class ImageBase {
  protected:
    std::size_t rows_, cols_;
    std::shared_ptr<std::vector<unsigned char>> data_;
    ImageColorMode color_mode_;
    ImageCompressMode cmp_mode_;

  public:
    /*!
     * \brief 空のImageを作成
     *
     */
    ImageBase()
        : rows_(0), cols_(0),
          data_(std::make_shared<std::vector<unsigned char>>()),
          color_mode_(ImageColorMode::gray), cmp_mode_(ImageCompressMode::raw) {
    }
    ImageBase(int rows, int cols,
              std::shared_ptr<std::vector<unsigned char>> data,
              ImageColorMode color_mode = ImageColorMode::bgr,
              ImageCompressMode cmp_mode = ImageCompressMode::raw)
        : rows_(rows), cols_(cols), data_(data), color_mode_(color_mode),
          cmp_mode_(cmp_mode) {
        if (cmp_mode == ImageCompressMode::raw &&
            rows * cols * channels() != data->size()) {
            throw std::invalid_argument("data size does not match");
        }
    }
    /*!
     * \brief 生画像データの配列からImageを取得
     *
     * dataから rows * cols * channels バイトがコピーされる
     *
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * \param data 画像データ
     * \param color_mode データの構造を指定
     * (デフォルトはOpenCVのBGR, uint8*3バイト)
     *
     */
    ImageBase(int rows, int cols, const void *data,
              ImageColorMode color_mode = ImageColorMode::bgr)
        : rows_(rows), cols_(cols), color_mode_(color_mode),
          cmp_mode_(ImageCompressMode::raw) {
        data_ = std::make_shared<std::vector<unsigned char>>(
            static_cast<const unsigned char *>(data),
            static_cast<const unsigned char *>(data) +
                rows * cols * channels());
    }

    /*!
     * \brief 画像が空かどうかを返す
     *
     * \return dataPtr()->size() == 0
     *
     */
    bool empty() const { return data_->size() == 0; }
    /*!
     * \brief 画像の幅
     *
     */
    std::size_t rows() const { return rows_; }
    /*!
     * \brief 画像の高さ
     *
     */
    std::size_t cols() const { return cols_; }
    /*!
     * \brief 1ピクセル当たりのデータサイズ(byte数)を取得
     *
     * \return 1, 3, or 4
     *
     */
    std::size_t channels() const {
        switch (color_mode_) {
        case ImageColorMode::gray:
            return 1;
        case ImageColorMode::bgr:
        case ImageColorMode::rgb:
            return 3;
        case ImageColorMode::bgra:
        case ImageColorMode::rgba:
            return 4;
        default:
            throw std::invalid_argument("unknown color format");
        }
    }
    /*!
     * \brief 色の並び順 (生画像データの場合)
     *
     * compress_modeがrawでない場合意味を持たない。
     *
     */
    ImageColorMode color_mode() const { return color_mode_; }
    /*!
     * \brief 画像の圧縮モード
     *
     */
    ImageCompressMode compress_mode() const { return cmp_mode_; }
    std::shared_ptr<std::vector<unsigned char>> dataPtr() const {
        return data_;
    }
    /*!
     * \brief 画像データ
     *
     * \return compress_modeがrawの場合、rows * cols * channels
     * 要素の画像データ。 それ以外の場合、圧縮された画像のデータ
     *
     */
    const std::vector<unsigned char> &data() const { return *data_; }
    /*!
     * \brief 画像の要素にアクセス
     *
     * compress_modeがrawでない場合は正常にアクセスできない。
     */
    unsigned char at(std::size_t row, std::size_t col,
                     std::size_t ch = 0) const {
        return dataPtr()->at((row * cols() + col) * channels() + ch);
    }
};

#if WEBCFACE_USE_OPENCV
//! 画像データ
/*!
 * 自身の持つデータを参照するcv::Matを生成、保持する。
 */
class ImageWithCV : public ImageBase {
    cv::Mat mat_;

    int CvType() {
        switch (channels()) {
        case 3:
            return CV_8UC3;
        case 4:
            return CV_8UC4;
        case 1:
            return CV_8UC1;
        default:
            throw std::invalid_argument(
                "Number of channels must be 1, 3 or 4.");
        }
    }

  public:
    /*!
     * \brief 空のImageを作成
     *
     */
    ImageWithCV() = default;
    /*!
     * \brief 画像データからcv::Matを生成
     *
     * ほかのコンストラクタからもこれが呼ばれる
     *
     */
    ImageWithCV(const ImageBase &base) : ImageBase(base), mat_() {
        if (empty()) {
            // mat_ = empty
        } else if (cmp_mode_ == ImageCompressMode::raw) {
            mat_ = cv::Mat{static_cast<int>(rows_), static_cast<int>(cols_),
                           CvType(), &data_->at(0)};
        } else {
            mat_ = cv::imdecode(*data_, cv::IMREAD_COLOR);
            if (static_cast<int>(rows_) != mat_.rows ||
                static_cast<int>(cols_) != mat_.cols) {
                throw std::invalid_argument("data size does not match");
            }
        }
    }
    ImageWithCV(int rows, int cols,
                std::shared_ptr<std::vector<unsigned char>> data,
                ImageColorMode color_mode, ImageCompressMode cmp_mode)
        : ImageWithCV(ImageBase(rows, cols, data, color_mode, cmp_mode)) {}

    /*!
     * \brief 生画像データの配列からImageを取得
     *
     * dataから rows * cols * channels バイトがコピーされる
     *
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * \param data 画像データ
     * \param color_mode データの構造を指定
     * (デフォルトはOpenCVのBGR, uint8*3バイト)
     *
     */
    ImageWithCV(int rows, int cols, const void *data,
                ImageColorMode color_mode = ImageColorMode::bgr)
        : ImageWithCV(ImageBase(rows, cols, data, color_mode)) {}

    /*!
     * \brief cv::MatからImageを取得
     *
     * matのコピーを内部に保持し、またmatの中身のデータもコピーする
     *
     * \param mat 画像データ フォーマットはCV_8UC1, CV_8UC3, CV_8UC4のみ対応
     * \param color_mode 色のモードを指定 (デフォルトはBGRA)
     */
    ImageWithCV(const cv::Mat &mat,
                ImageColorMode color_mode = ImageColorMode::bgr)
        : ImageBase(), mat_(mat) {
        rows_ = mat.rows;
        cols_ = mat.cols;
        color_mode_ = color_mode;
        cmp_mode_ = ImageCompressMode::raw;
        if (mat.depth() != CV_8U) {
            throw std::invalid_argument(
                "webcface::ImageData supports CV_8UC1 (grayscale), CV_8UC3 "
                "(BGR) or CV_8UC4 (BGRA) format only.");
        }
        if (mat.channels() != static_cast<int>(channels())) {
            throw std::invalid_argument("color_mode does not match");
        }
        data_ = std::make_shared<std::vector<unsigned char>>();
        // https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
        if (mat.isContinuous()) {
            data_->assign(static_cast<unsigned char *>(mat.data),
                          static_cast<unsigned char *>(mat.data) +
                              mat.total() * mat.channels());
        } else {
            for (int i = 0; i < mat.rows; ++i) {
                data_->insert(data_->end(), mat.ptr<unsigned char>(i),
                              mat.ptr<unsigned char>(i) +
                                  mat.cols * mat.channels());
            }
        }
    }
    // operator cv::Mat &() { return mat_; }
    /*!
     * \brief cv::Matに変換した画像を返す
     *
     * 生画像の場合cv::Matの内部のデータは
     * ImageWithCV (=ImageFrame)が保持しているので、
     * ImageWithCVの一時オブジェクトからmatを取り出すことはできない。
     *
     */
    cv::Mat &mat() & { return mat_; }
};

using ImageFrame = ImageWithCV;

#else
using ImageFrame = ImageBase;
#endif

struct ImageReq {
    std::optional<int> rows, cols;
    std::optional<ImageColorMode> color_mode = ImageColorMode::bgra;
    ImageCompressMode cmp_mode = ImageCompressMode::raw;
    int quality = 0;
    std::optional<double> frame_rate;

    bool operator==(const ImageReq &rhs) const {
        return rows == rhs.rows && cols == rhs.cols &&
               color_mode == rhs.color_mode && cmp_mode == rhs.cmp_mode &&
               quality == rhs.quality;
    }
    bool operator!=(const ImageReq &rhs) const { return !(*this == rhs); }
};
} // namespace Common
} // namespace WEBCFACE_NS
