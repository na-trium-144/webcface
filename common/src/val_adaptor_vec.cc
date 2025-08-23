#include "webcface/common/val_adaptor_vec.h"

WEBCFACE_NS_BEGIN

ValAdaptorVector::ValAdaptorVector() = default;
ValAdaptorVector::ValAdaptorVector(const ValAdaptor &val) : vec({val}) {}
ValAdaptorVector &ValAdaptorVector::operator=(const ValAdaptor &val) {
    vec = std::vector<ValAdaptor>{val};
    return *this;
}
ValAdaptorVector::ValAdaptorVector(std::vector<ValAdaptor> init)
    : vec(std::move(init)) {}
ValAdaptorVector &ValAdaptorVector::operator=(std::vector<ValAdaptor> init) {
    vec = std::move(init);
    return *this;
}

ValAdaptorVector::ValAdaptorVector(StringInitializer str)
    : ValAdaptorVector(ValAdaptor(std::move(str))) {}
ValAdaptorVector &ValAdaptorVector::operator=(StringInitializer str) {
    return *this = ValAdaptor(std::move(str));
}

const ValAdaptor &ValAdaptorVector::get() const {
    if (vec.size() == 0) {
        vec.resize(1);
        return at(0);
    } else if (vec.size() == 1) {
        return at(0);
    } else {
        throw std::invalid_argument("expected single value, but got list of " +
                                    std::to_string(vec.size()) + " elements");
    }
}
ValType ValAdaptorVector::valType() const {
    if (vec.size() == 1) {
        return vec[0].valType();
    } else if (vec.size() == 0) {
        return ValType::none_;
    } else {
        return ValType::vector_;
    }
}

const ValAdaptor &ValAdaptorVector::at(std::size_t index) const {
    return vec.at(index);
}
std::size_t ValAdaptorVector::size() const { return vec.size(); }

bool ValAdaptorVector::operator==(const ValAdaptorVector &other) const {
    return this->vec == other.vec;
}

WEBCFACE_NS_END
