#pragma once
#include <vector>
#include <memory>
#include <stdexcept>

// todo: cmakeなしでヘッダー読んだときにopencvの有無を判別する
#if WEBCFACE_USE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

namespace webcface {
inline namespace Common {


enum class ImageCompressMode {
    raw = 0,
    jpeg = 1,
    webp = 2,
    png = 3,
};

//! 画像データ
/*!
 * 8bitのグレースケール, BGR, BGRAフォーマットのみを扱う
 */
class ImageBase {
  protected:
    int rows_, cols_, channels_;
    std::shared_ptr<std::vector<unsigned char>> data_;
    ImageCompressMode mode_;

  public:
    ImageBase()
        : rows_(0), cols_(0), channels_(1),
          data_(std::make_shared<std::vector<unsigned char>>()),
          mode_(ImageCompressMode::raw) {}
    ImageBase(int rows, int cols, int channels,
              std::shared_ptr<std::vector<unsigned char>> data,
              ImageCompressMode mode)
        : rows_(rows), cols_(cols), channels_(channels), data_(data),
          mode_(mode) {
        if (mode == ImageCompressMode::raw &&
            rows * cols * channels != data->size()) {
            throw std::invalid_argument("data size does not match");
        }
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int channels() const { return channels_; }
    ImageCompressMode mode() const { return mode_; }
    std::shared_ptr<std::vector<unsigned char>> dataPtr() const {
        return data_;
    }
    const std::vector<unsigned char> &data() const { return *data_; }
    unsigned char at(int row, int col, int ch = 0) const {
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
        switch (channels_) {
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
    ImageWithCV() = default;
    ImageWithCV(const ImageBase &base) : ImageBase(base) {
        if (mode_ == ImageCompressMode::raw) {
            mat_ = cv::Mat{rows_, cols_, CvType(), &data_->at(0)};
        } else {
            mat_ = cv::imdecode(*data_, channels_ == 1
                                                   ? cv::IMREAD_GRAYSCALE
                                                   : cv::IMREAD_COLOR);
            if (rows_ != mat_.rows || cols_ != mat_.cols ||
                channels_ != mat_.channels()) {
                throw std::invalid_argument("data size does not match");
            }
        }
    }
    ImageWithCV(int rows, int cols, int channels,
                std::shared_ptr<std::vector<unsigned char>> data,
                ImageCompressMode mode)
        : ImageWithCV(ImageBase(rows, cols, channels, data, mode)) {}

    ImageWithCV(const cv::Mat &mat) : ImageBase(), mat_(mat) {
        rows_ = mat.rows;
        cols_ = mat.cols;
        channels_ = mat.channels();
        mode_ = ImageCompressMode::raw;
        if (mat.depth() != CV_8U ||
            channels_ != 1 && channels_ != 3 && channels_ != 4) {
            throw std::invalid_argument(
                "webcface::ImageData supports CV_8UC1 (grayscale), CV_8UC3 "
                "(BGR) or CV_8UC4 (BGRA) format only.");
        }
        data_ = std::make_shared<std::vector<unsigned char>>();
        // https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
        if (mat.isContinuous()) {
            data_->assign(reinterpret_cast<unsigned char *>(mat.data),
                          reinterpret_cast<unsigned char *>(mat.data) +
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
    cv::Mat &mat() & { return mat_; }
};

using ImageFrame = ImageWithCV;

#else
using ImageFrame = ImageBase;
#endif

struct ImageReq {
    std::optional<int> rows, cols;
    int channels = 3;
    ImageCompressMode mode = ImageCompressMode::raw;
    int quality = 0;

    bool operator==(const ImageReq &rhs) const {
        return rows == rhs.rows && cols == rhs.cols &&
               channels == rhs.channels && mode == rhs.mode &&
               quality == rhs.quality;
    }
    bool operator!=(const ImageReq &rhs) const { return !(*this == rhs); }
};
} // namespace Common
} // namespace webcface