#include <webcface/encoding/val_adaptor.h>

WEBCFACE_NS_BEGIN
inline namespace encoding {
ValAdaptor::ValAdaptor() : type(ValType::none_) {}

ValAdaptor::ValAdaptor(const SharedString &str)
    : as_str(str), type(ValType::string_) {}
ValAdaptor &ValAdaptor::operator=(const SharedString &str) {
    as_str = str;
    type = ValType::string_;
    return *this;
}

ValAdaptor::ValAdaptor(std::u8string_view str)
    : as_str(str), type(ValType::string_) {}
ValAdaptor &ValAdaptor::operator=(std::u8string_view str) {
    as_str = SharedString(str);
    type = ValType::string_;
    return *this;
}

ValAdaptor::ValAdaptor(std::string_view str)
    : as_str(str), type(ValType::string_) {}
ValAdaptor &ValAdaptor::operator=(std::string_view str) {
    as_str = SharedString(str);
    type = ValType::string_;
    return *this;
}

ValAdaptor::ValAdaptor(std::wstring_view str)
    : as_str(str), type(ValType::string_) {}
ValAdaptor &ValAdaptor::operator=(std::wstring_view str) {
    as_str = SharedString(str);
    type = ValType::string_;
    return *this;
}

ValAdaptor::ValAdaptor(bool value)
    : as_val(static_cast<std::int64_t>(value)), type(ValType::bool_) {}
ValAdaptor &ValAdaptor::operator=(bool v) {
    as_val.emplace<INT64V>(v);
    type = ValType::bool_;
    return *this;
}

ValAdaptor::ValAdaptor(std::int64_t value)
    : as_val(value), type(ValType::int_) {}
ValAdaptor &ValAdaptor::operator=(std::int64_t v) {
    as_val.emplace<INT64V>(v);
    type = ValType::int_;
    return *this;
}

ValAdaptor::ValAdaptor(double value) : as_val(value), type(ValType::float_) {}
ValAdaptor &ValAdaptor::operator=(double v) {
    as_val.emplace<DOUBLEV>(v);
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

const std::string &ValAdaptor::asStringRef() const {
    if (as_str.empty() && valType() != ValType::none_ &&
        valType() != ValType::string_) {
        if (as_val.index() == DOUBLEV) {
            as_str = SharedString(std::to_string(std::get<DOUBLEV>(as_val)));
        } else {
            as_str = SharedString(std::to_string(std::get<INT64V>(as_val)));
        }
    }
    return as_str.decode();
}

const std::wstring &ValAdaptor::asWStringRef() const {
    if (as_str.empty() && valType() != ValType::none_ &&
        valType() != ValType::string_) {
        if (as_val.index() == DOUBLEV) {
            as_str = SharedString(std::to_wstring(std::get<DOUBLEV>(as_val)));
        } else {
            as_str = SharedString(std::to_wstring(std::get<INT64V>(as_val)));
        }
    }
    return as_str.decodeW();
}

const std::u8string &ValAdaptor::asU8StringRef() const {
    if (as_str.empty() && valType() != ValType::none_ &&
        valType() != ValType::string_) {
        if (as_val.index() == DOUBLEV) {
            as_str = SharedString(std::to_string(std::get<DOUBLEV>(as_val)));
        } else {
            as_str = SharedString(std::to_string(std::get<INT64V>(as_val)));
        }
    }
    return as_str.u8String();
}

double ValAdaptor::asDouble() const {
    if (type == ValType::string_) {
        try {
            return std::stod(asStringRef());
        } catch (...) {
            return 0;
        }
    } else {
        switch (as_val.index()) {
        case DOUBLEV:
            return std::get<DOUBLEV>(as_val);
        default:
            return static_cast<double>(std::get<INT64V>(as_val));
        }
    }
}
int ValAdaptor::asInt() const {
    if (type == ValType::string_) {
        try {
            return std::stoi(asStringRef());
        } catch (...) {
            return 0;
        }
    } else {
        switch (as_val.index()) {
        case DOUBLEV:
            return static_cast<int>(std::get<DOUBLEV>(as_val));
        default:
            return static_cast<int>(std::get<INT64V>(as_val));
        }
    }
}
long long ValAdaptor::asLLong() const {
    if (type == ValType::string_) {
        try {
            return std::stoll(asStringRef());
        } catch (...) {
            return 0;
        }
    } else {
        switch (as_val.index()) {
        case DOUBLEV:
            return static_cast<long long>(std::get<DOUBLEV>(as_val));
        default:
            return static_cast<long long>(std::get<INT64V>(as_val));
        }
    }
}

bool ValAdaptor::asBool() const {
    if (type == ValType::string_) {
        return !empty();
    } else {
        switch (as_val.index()) {
        case DOUBLEV:
            return std::get<DOUBLEV>(as_val) != 0;
        default:
            return std::get<INT64V>(as_val) != 0;
        }
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
        return this->asU8StringRef() == other.asU8StringRef();
    }
}
} // namespace encoding
WEBCFACE_NS_END
