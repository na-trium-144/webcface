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

/**
 * \brief shared_ptrで管理されているdoubleのvector
 * \since ver2.10
 * 
 * * 基本的にはvectorと同じだが、shared_ptrで管理されているため、コピーしたら要素は共有される。
 * * 要素数が最初から1の場合メモリの動的確保をしない。
 * * 要素数0にはならず、その場合要素数1で値が0となる。
 * 
 */
class NumVector {
    std::variant<std::shared_ptr<std::vector<double>>, double> data;

public:
    NumVector(double v = 0): data(v){}
    NumVector(std::vector<double> vec) {
        assign(std::move(vec));
    }
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    NumVector(const R &range): NumVector(traits::arrayLikeToVector(range)) {}

    void assign(double v){
        (*this)[0] = v;
    }
    NumVector &operator=(double v){
        assign(v);
        return *this;
    }
    void assign(std::vector<double> vec) {
        data.emplace<std::shared_ptr<std::vector<double>>>(std::make_shared<std::vector<double>>(std::move(vec)));
    }
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

    double &operator[](std::size_t index){
        switch(data.index()){
        case 0:
            return (*std::get<0>(data))[index];
        case 1:
            return std::get<1>(data);
        }
    }
    const double &operator[](std::size_t index) const {
        switch(data.index()){
        case 0:
            return (*std::get<0>(data))[index];
        case 1:
            return std::get<1>(data);
        }
    }
    double &at(std::size_t index){
        switch(data.index()){
        case 0:
            if(index >= std::get<0>(data)->size()){
                throw std::out_of_range("NumVector::at() got index " + std::to_string(index) + ", but size is " + std::to_string(std::get<0>(data)->size()));
            }
            return (*std::get<0>(data))[index];
        case 1:
            if(index >= 1){
                throw std::out_of_range("NumVector::at() got index " + std::to_string(index) + ", but size is 1");
            }
            return std::get<1>(data);
        }
    }
    const double &at(std::size_t index) const{
        switch(data.index()){
        case 0:
            if(index >= std::get<0>(data)->size()){
                throw std::out_of_range("NumVector::at() got index " + std::to_string(index) + ", but size is " + std::to_string(std::get<0>(data)->size()));
            }
            return (*std::get<0>(data).vec)[index];
        case 1:
            if(index >= 1){
                throw std::out_of_range("NumVector::at() got index " + std::to_string(index) + ", but size is 1");
            }
            return std::get<1>(data);
        }
    }

    double *data() {
        return &(*this[0]);
    }
    const double *data() const{
        return &(*this[0]);
    }
    double *begin(){
        return &(*this[0]);
    }
    double *end(){
        return begin() + size();
    }
    const double *begin() const{
        return &(*this[0]);
    }
    const double *end() const{
        return begin() + size();
    }
    const double *cbegin() const{
        return &(*this[0]);
    }
    const double *cend() const{
        return begin() + size();
    }

    void resize(std::size_t new_size){
        if(new_size == 0){
            new_size = 1;
        }
        switch(data.index()){
        case 0:{
            std::get<0>(data)->resize(new_size);
            break;
        }
        case 1:{
            if(new_size >= 2){
                std::vector<double> vec(new_size);
                vec[0] = std::get<1>(data);
                data.emplace<std::shared_ptr<std::vector<double>>>(std::make_shared<std::vector<double>>(std::move(vec)));
            }
            break;
        }
        }
    }
    void push_back(double v){
        switch(data.index()){
        case 0:{
            std::get<0>(data)->push_back(v);
            break;
        }
        case 1:{
            std::vector<double> vec = {std::get<1>(data), v};
            data.emplace<std::shared_ptr<std::vector<double>>>(std::make_shared<std::vector<double>>(std::move(vec)));
            break;
        }
        }
    }
    std::size_t size() const{
        switch(data.index()){
        case 0:
            return std::get<0>(data)->size();
        case 1:
            return 1;
        }
    }
};

WEBCFACE_NS_END
