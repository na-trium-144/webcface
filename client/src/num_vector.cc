#include <webcface/num_vector.h>
#include <webcface/common/internal/message/base.h>

WEBCFACE_NS_BEGIN

NumVector::NumVector(const message::NumVector &msg) : data_(msg.data) {}
NumVector::operator message::NumVector() const {
    return message::NumVector{data_};
}

NumVector::NumVector(double v) : data_(v) {}
void MutableNumVector::assign(double v) { data_.emplace<double>(v); }

NumVector::NumVector(std::vector<double> vec)
    : data_(std::make_shared<std::vector<double>>(std::move(vec))) {}
void MutableNumVector::assign(std::vector<double> vec) {
    data_.emplace<std::shared_ptr<std::vector<double>>>(
        std::make_shared<std::vector<double>>(std::move(vec)));
}

NumVector::operator const std::vector<double>&() const {
    if(data_.index() == 0){
        data_.emplace<std::shared_ptr<std::vector<double>>>(std::make_shared<std::vector<double>>(std::vector<double>{at(0)}));
    }
    return *std::get<1>(data_);
}

double &MutableNumVector::at(std::size_t index) {
    if (index >= size()) {
        throw OutOfRange("NumVector::at() got index " + std::to_string(index) +
                         ", but size is " + std::to_string(size()));
    }
    switch (data_.index()) {
    case 0:
        return std::get<0>(data_);
    case 1:
    default:
        return (*std::get<1>(data_))[index];
    }
}
const double &NumVector::at(std::size_t index) const {
    if (index >= size()) {
        throw OutOfRange("NumVector::at() got index " + std::to_string(index) +
                         ", but size is " + std::to_string(size()));
    }
    switch (data_.index()) {
    case 0:
        return std::get<0>(data_);
    case 1:
    default:
        return (*std::get<1>(data_))[index];
    }
}

void MutableNumVector::resize(std::size_t new_size) {
    if (new_size == 0) {
        new_size = 1;
    }
    switch (data_.index()) {
    case 0: {
        if (new_size >= 2) {
            std::vector<double> vec(new_size);
            vec[0] = std::get<0>(data_);
            data_.emplace<std::shared_ptr<std::vector<double>>>(
                std::make_shared<std::vector<double>>(std::move(vec)));
        }
        break;
    }
    case 1:
    default: {
        std::get<1>(data_)->resize(new_size);
        break;
    }
    }
}
void MutableNumVector::push_back(double v) {
    switch (data_.index()) {
    case 0: {
        std::vector<double> vec = {std::get<0>(data_), v};
        data_.emplace<std::shared_ptr<std::vector<double>>>(
            std::make_shared<std::vector<double>>(std::move(vec)));
        break;
    }
    case 1:
    default: {
        std::get<1>(data_)->push_back(v);
        break;
    }
    }
}
std::size_t NumVector::size() const {
    switch (data_.index()) {
    case 0:
        return 1;
    case 1:
    default:
        return std::get<1>(data_)->size();
    }
}

bool NumVector::operator==(const NumVector &other) const {
    if (data_.index() == 0 && other.data_.index() == 0 &&
        std::get<0>(data_) == std::get<0>(other.data_)) {
        return true;
    }
    if (data_.index() == 1 && other.data_.index() == 1 &&
        std::get<1>(data_) == std::get<1>(other.data_) // ポインタ比較
    ) {
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
