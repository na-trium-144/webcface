#pragma once
#include <memory>
#include <variant>
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
    struct SharedVector{
        std::size_t size, cap;
        std::shared_ptr<double[]> vec;
    };
    std::variant<SharedVector, double> data;

public:
    NumVector(double v = 0): data(v){}
    template <typename It>
    NumVector(It begin, It end) {
        assign(begin, end);
    }
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    NumVector(const R &range): NumVector(std::begin(range), std::end(range)) {}
    NumVector(std::initializer_list<double> range): NumVector(std::begin(range), std::end(range)) {}

    void assign(double v){
        (*this)[0] = v;
    }
    NumVector &operator=(double v){
        assign(v);
        return *this;
    }
    template <typename It>
    void assign(It begin, It end) {
        std::size_t size = end - begin;
        if(size == 0){
            data.emplace<double>(0);
        }else if(size == 1){
            data.emplace<double>(*begin);
        }else{
            SharedVector sv(size, size, new double[size])
            auto it = begin;
            for(std::size_t i = 0; it != end && i < size; it++, i++){
                sv.vec[i] = *it;
            }
            data.emplace<SharedVector>(std::move(sv));
        }
    }
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    void assign(const R &range) {
        assign(std::begin(range), std::end(range));
    }
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    NumVector &operator=(const R &range) {
        assign(range);
        return *this;
    }
    void assign(std::initializer_list<double> range) {
        assign(std::begin(range), std::end(range));
    }
    NumVector &operator=(std::initializer_list<double> range) {
        assign(range);
        return *this;
    }

    double &operator[](std::size_t index){
        switch(data.index()){
        case 0:
            return std::get<0>(data).vec[index];
        case 1:
            return std::get<1>(data);
        }
    }
    const double &operator[](std::size_t index) const {
        switch(data.index()){
        case 0:
            return std::get<0>(data).vec[index];
        case 1:
            return std::get<1>(data);
        }
    }
    double &at(std::size_t index){
        switch(data.index()){
        case 0:
            if(index >= std::get<0>(data).size){
                throw std::out_of_range("NumVector::at() got index " + std::to_string(index) + ", but size is " + std::to_string(std::get<0>(data).size));
            }
            return std::get<0>(data).vec[index];
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
            if(index >= std::get<0>(data).size){
                throw std::out_of_range("NumVector::at() got index " + std::to_string(index) + ", but size is " + std::to_string(std::get<0>(data).size));
            }
            return std::get<0>(data).vec[index];
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
            auto&sv = std::get<0>(data);
            if(new_size <= sv.cap){
                for(std::size_t i = sv.size; i < new_size; i++){
                    sv.vec[i] = 0;
                }
                sv.size = new_size;
            }else{
                std::shared_ptr<double[]> new_vec(new double[new_size]);
                for(std::size_t i = 0; i < new_size; i++){
                    new_vec[i] = i < sv.size ? sv.vec[i] : 0;
                }
                data.emplace<SharedVector>(new_size, new_size, std::move(new_vec));
            }
            break;
        }
        case 1:{
            if(new_size >= 2){
                SharedVector sv{new_size, new_size, new double[new_size]};
                sv.vec[0] = std::get<1>(data);
                for(std::size_t i = 1; i < new_size; i++){
                    sv.vec[i] = 0;
                }
                data.emplace<SharedVector>(std::move(sv));
            }
            break;
        }
        }
    }
    void push_back(double v){
        switch(data.index()){
        case 0:{
            auto&sv = std::get<0>(data);
            if(sv.size + 1 < sv.cap){
                sv.vec[sv.size] = v;
                sv.size++;
            }else{
                std::shared_ptr<double[]> new_vec(new double[sv.cap * 2]);
                for(std::size_t i = 0; i < sv.size; i++){
                    new_vec[i] = sv.vec[i];
                }
                new_vec[sv.size] = v;
                data.emplace<SharedVector>(sv.size + 1, sv.cap * 2, std::move(new_vec));
            }
            break;
        }
        case 1:{
            std::size_t new_cap = 4;
            SharedVector sv{2, new_cap, new double[new_cap]};
            sv.vec[0] = std::get<1>(data);
            sv.vec[1] = v;
            data.emplace<SharedVector>(std::move(sv));
            break;
        }
        }
    }
    std::size_t size() const{
        switch(data.index()){
        case 0:
            return std::get<0>(data).size;
        case 1:
            return 1;
        }
    }
};

WEBCFACE_NS_END
