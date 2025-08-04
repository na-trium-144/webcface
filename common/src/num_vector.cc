#include <webcface/common/num_vector.h>
#include <stdexcept>
#include <string>

WEBCFACE_NS_BEGIN

NumVector::NumVector(double v) : data_(), first_(v) {}
void MutableNumVector::assign(double v) { first_ = v; data_ = nullptr; }

NumVector::NumVector(std::vector<double> vec)
    : data_(std::make_shared<std::vector<double>>(std::move(vec))), first_(data_->at(0)) {}
void MutableNumVector::assign(std::vector<double> vec) {
    data_ = std::make_shared<std::vector<double>>(std::move(vec));
    first_ = data_->at(0);
}

NumVector::operator const std::vector<double>&() const {
    if(!data_){
        data_ = std::make_shared<std::vector<double>>(std::vector<double>{first_});
    }
    return *data_;
}

double &MutableNumVector::at(std::size_t index) {
    if (index >= size()) {
        throw std::out_of_range("NumVector::at() got index " + std::to_string(index) +
                         ", but size is " + std::to_string(size()));
    }
    if(data_){
        return data_->at(index);
    }else{
        return first_;
    }
}
const double &NumVector::at(std::size_t index) const {
    if (index >= size()) {
        throw std::out_of_range("NumVector::at() got index " + std::to_string(index) +
                         ", but size is " + std::to_string(size()));
    }
    if(data_){
        return data_->at(index);
    }else{
        return first_;
    }
}

void MutableNumVector::resize(std::size_t new_size) {
    if (new_size == 0) {
        new_size = 1;
    }
    if(data_){
        data_->resize(new_size);
    }else{
        std::vector<double> vec(new_size);
        vec[0] = first_;
        data_ = std::make_shared<std::vector<double>>(std::move(vec));
    }
}
void MutableNumVector::push_back(double v) {
    if(data_){
        data_->push_back(v);
    }else{
        std::vector<double> vec = {first_, v};
        data_ = std::make_shared<std::vector<double>>(std::move(vec));
    }
}
std::size_t NumVector::size() const {
    if(data_){
        return data_->size();
    }else{
        return 1;
    }
}

bool NumVector::operator==(const NumVector &other) const {
    if (!data_ && !other.data_ &&
        first_ == other.first_) {
        return true;
    }
    if (data_ && other.data_ && data_ == other.data_ /* ポインタ比較 */) {
        return true;
    }
    if (size() != other.size()) {
        return false;
    }
    for (std::size_t i = 0; i < size(); i++) {
        if (at(i) != other.at(i)) {
            return false;
        }
    }
    return true;
}
WEBCFACE_NS_END
