#pragma once
#include <vector>
#include <memory>
#include <stdexcept>

// todo: cmakeなしでヘッダー読んだときにopencvの有無を判別する
#if WEBCFACE_USE_OPENCV
#include <opencv2/core.hpp>
#endif

namespace webcface {
inline namespace Common {

enum class ImageColorType {
    null = 0,
    BGR = 1,  //!< 8bit x 3
    BGRA = 2, //!< 8bit x 4
    RGB = 3,  //!< 8bit x 3
    RGBA = 4, //!< 8bit x 4
    GRAY8 = 5,
};

//! 画像データ
/*!
 * cv::Matと異なり、数種類の色の表現方法の判別はする。
 */
class ImageBase {
  protected:
    int rows_, cols_;
    ImageColorType type_;
    std::shared_ptr<std::vector<char>> data_;

  public:
    ImageBase()
        : rows_(0), cols_(0), type_(ImageColorType::null),
          data_(std::make_shared<std::vector<char>>()) {}
    ImageBase(int rows, int cols, ImageColorType type,
              std::shared_ptr<std::vector<char>> data)
        : rows_(rows), cols_(cols), type_(type), data_(data) {
        if (rows * cols * elementSize() != data->size()) {
            throw std::invalid_argument("data size does not match");
        }
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    ImageColorType type() const { return type_; }
    int elementSize() const {
        switch (type()) {
        case ImageColorType::BGR:
        case ImageColorType::RGB:
            return 3;
        case ImageColorType::BGRA:
        case ImageColorType::RGBA:
            return 4;
        case ImageColorType::GRAY8:
            return 1;
        default:
            throw std::invalid_argument("Unsupported Image Color Type");
        }
    }
    std::shared_ptr<std::vector<char>> data() const { return data_; }
    template <typename T>
    T &at(int row, int col, int dim = 0) {
        return *reinterpret_cast<T *>(
            &data()->at((row * cols() + col) * elementSize() + dim));
    }
    template <typename T>
    const T &at(int row, int col, int dim = 0) const {
        return *reinterpret_cast<T *>(
            &data()->at((row * cols() + col) * elementSize() + dim));
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
        switch (type_) {
        case ImageColorType::BGR:
        case ImageColorType::RGB:
            return CV_8UC3;
        case ImageColorType::BGRA:
        case ImageColorType::RGBA:
            return CV_8UC4;
        case ImageColorType::GRAY8:
            return CV_8UC1;
        default:
            throw std::invalid_argument("Unsupported Image Color Type");
        }
    }

  public:
    ImageWithCV() = default;
    ImageWithCV(const ImageBase &base)
        : ImageBase(base), mat_(rows_, cols_, CvType(), &data_->at(0)) {}
    ImageWithCV(int rows, int cols, ImageColorType type,
                std::shared_ptr<std::vector<char>> data)
        : ImageBase(rows, cols, type, data),
          mat_(rows, cols, CvType(), &data->at(0)) {}
    operator cv::Mat &() { return mat_; }
    cv::Mat &mat() { return mat_; }
};

using Image = ImageWithCV;

#else
using Image = ImageBase;
#endif

} // namespace Common
} // namespace webcface