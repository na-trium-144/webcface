#pragma once
#include <memory>
#include <vector>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
/**
 * \brief shared_ptrで管理されているdoubleのvector
 * \since ver2.10
 *
 * * 基本的にはvectorと同じだが、shared_ptrで管理されているため、コピーしたら要素は共有される。
 * * 要素数が最初から1の場合メモリの動的確保をしない。
 * * 要素数0にはならず、その場合要素数1で値が0となる。
 *
 */
class WEBCFACE_DLL NumVector {
  protected:
    mutable std::shared_ptr<std::vector<double>> data_;
    // data_がnullptrでない場合、first_の値がdata_[0]と同期している保証はない
    double first_;

  public:
    NumVector(double v = 0);
    NumVector(std::vector<double> vec);

    /*!
     * 参照はこのNumVectorが破棄されるまで有効
     *
     */
    operator const std::vector<double>&() const;

    const double &operator[](std::size_t index) const { return at(index); }
    const double &at(std::size_t index) const;

    const double *data() const { return &at(0); }
    const double *begin() const { return &at(0); }
    const double *end() const { return begin() + size(); }
    const double *cbegin() const { return &at(0); }
    const double *cend() const { return begin() + size(); }

    std::size_t size() const;

    bool operator==(const NumVector &other) const;
    bool operator!=(const NumVector &other) const { return !(*this == other); }
};

/**
 * \brief shared_ptrで管理されているdoubleのvector
 * \since ver2.10
 *
 */
class WEBCFACE_DLL MutableNumVector : public NumVector {
  public:
    MutableNumVector(double v = 0) : NumVector(v){}
    MutableNumVector(std::vector<double> vec): NumVector(std::move(vec)) {}

    void assign(double v);
    NumVector &operator=(double v) {
        assign(v);
        return *this;
    }
    void assign(std::vector<double> vec);

    operator const std::vector<double>&() const = delete;

    using NumVector::operator[];
    double &operator[](std::size_t index) { return at(index); }
    using NumVector::at;
    double &at(std::size_t index);

    using NumVector::data;
    double *data() { return &at(0); }
    using NumVector::begin;
    double *begin() { return &at(0); }
    using NumVector::end;
    double *end() { return begin() + size(); }

    void resize(std::size_t new_size);
    void push_back(double v);
};

WEBCFACE_NS_END
