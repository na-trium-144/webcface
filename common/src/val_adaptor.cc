#include "webcface/common/val_adaptor.h"
#include <cassert>

WEBCFACE_NS_BEGIN
const ValAdaptor &ValAdaptor::emptyVal() {
    static ValAdaptor empty;
    return empty;
}

ValAdaptor::ValAdaptor(const SharedString &str)
    : as_str(str), type(ValType::string_) {}
ValAdaptor &ValAdaptor::operator=(const SharedString &str) {
    as_str = str;
    as_double = 0;
    as_int = 0;
    type = ValType::string_;
    return *this;
}

ValAdaptor::ValAdaptor(bool value) : as_int(value), type(ValType::bool_) {}
ValAdaptor &ValAdaptor::operator=(bool v) {
    as_int = v;
    as_str = nullptr;
    as_double = 0;
    type = ValType::bool_;
    return *this;
}

ValAdaptor::ValAdaptor(std::int64_t value)
    : as_int(value), type(ValType::int_) {}
ValAdaptor &ValAdaptor::operator=(std::int64_t v) {
    as_int = v;
    as_str = nullptr;
    as_double = 0;
    type = ValType::int_;
    return *this;
}

ValAdaptor::ValAdaptor(double value)
    : as_double(value), type(ValType::float_) {}
ValAdaptor &ValAdaptor::operator=(double v) {
    as_double = v;
    as_str = nullptr;
    as_int = 0;
    type = ValType::float_;
    return *this;
}

bool ValAdaptor::empty() const {
    if (type == ValType::none_ || type == ValType::string_) {
        return as_str.empty();
    } else {
        return false;
    }
}

void ValAdaptor::initString() const {
    if (as_str.empty() && valType() != ValType::none_ &&
        valType() != ValType::string_) {
        assert(!(as_double != 0 && as_int != 0));
        if (as_double != 0) {
            as_str = SharedString::encode(std::to_string(as_double));
        } else {
            as_str = SharedString::encode(std::to_string(as_int));
        }
    }
}
void ValAdaptor::initWString() const {
    if (as_str.empty() && valType() != ValType::none_ &&
        valType() != ValType::string_) {
        assert(!(as_double != 0 && as_int != 0));
        if (as_double != 0) {
            as_str = SharedString::encode(std::to_wstring(as_double));
        } else {
            as_str = SharedString::encode(std::to_wstring(as_int));
        }
    }
}

void ValAdaptor::initU8String() const {
    if (as_str.empty() && valType() != ValType::none_ &&
        valType() != ValType::string_) {
        assert(!(as_double != 0 && as_int != 0));
        if (as_double != 0) {
            as_str = SharedString::fromU8String(std::to_string(as_double));
        } else {
            as_str = SharedString::fromU8String(std::to_string(as_int));
        }
    }
}

double ValAdaptor::asDouble() const {
    if (type == ValType::string_) {
        return std::atof(asU8CStr());
    } else {
        assert(!(as_double != 0 && as_int != 0));
        if(as_double != 0) {
            return as_double;
        }else{
            return static_cast<double>(as_int);
        }
    }
}
int ValAdaptor::asInt() const {
    if (type == ValType::string_) {
        return std::atoi(asU8CStr());
    } else {
        assert(!(as_double != 0 && as_int != 0));
        if(as_double != 0) {
            return static_cast<int>(as_double);
        }else{
            return static_cast<int>(as_int);
        }
    }
}
long long ValAdaptor::asLLong() const {
    if (type == ValType::string_) {
        return std::atoll(asU8CStr());
    } else {
        assert(!(as_double != 0 && as_int != 0));
        if(as_double != 0) {
            return static_cast<long long>(as_double);
        }else{
            return as_int;
        }
    }
}

bool ValAdaptor::asBool() const {
    if (type == ValType::string_) {
        return !empty();
    } else {
        assert(!(as_double != 0 && as_int != 0));
        return as_double != 0 || as_int != 0;
    }
}

bool ValAdaptor::operator==(const ValAdaptor &other) const {
    if (type == ValType::double_ || other.type == ValType::double_) {
        return this->asDouble() == other.asDouble();
    } else if (type == ValType::int_ || other.type == ValType::int_) {
        return this->asLLong() == other.asLLong();
    } else if (type == ValType::bool_ || other.type == ValType::bool_) {
        return this->asBool() == other.asBool();
    } else {
        return this->as_str == other.as_str;
    }
}
WEBCFACE_NS_END
