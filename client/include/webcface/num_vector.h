#pragma once
#include <memory>
#include <variant>
#include <vector>
#include "./array_like.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct NumVector;
}
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
    std::variant<double, std::shared_ptr<std::vector<double>>> data_;

  public:
    NumVector(double v = 0);
    NumVector(std::vector<double> vec);
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    NumVector(const R &range) : NumVector(traits::arrayLikeToVector(range)) {}

    NumVector(const message::NumVector &msg);
    operator message::NumVector() const;

    void assign(double v);
    NumVector &operator=(double v) {
        assign(v);
        return *this;
    }
    void assign(std::vector<double> vec);
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    void assign(const R &range) {
        assign(traits::arrayLikeToVector(range));
    }
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    NumVector &operator=(const R &range) {
        assign(range);
        return *this;
    }

    double &operator[](std::size_t index) { return at(index); }
    const double &operator[](std::size_t index) const { return at(index); }
    double &at(std::size_t index);
    const double &at(std::size_t index) const;

    double *data() { return &at(0); }
    const double *data() const { return &at(0); }
    double *begin() { return &at(0); }
    double *end() { return begin() + size(); }
    const double *begin() const { return &at(0); }
    const double *end() const { return begin() + size(); }
    const double *cbegin() const { return &at(0); }
    const double *cend() const { return begin() + size(); }

    void resize(std::size_t new_size);
    void push_back(double v);
    std::size_t size() const;

    bool operator==(const NumVector &other) const;
    bool operator!=(const NumVector &other) const { return !(*this == other); }
};

WEBCFACE_NS_END
