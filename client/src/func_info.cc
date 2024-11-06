#include "webcface/internal/func_internal.h"
#include "webcface/message/func.h"

WEBCFACE_NS_BEGIN

Arg::Arg(ValType type) : msg_data(), type_(type) {}
Arg::Arg(const std::shared_ptr<message::Arg> &msg_data)
    : msg_data(msg_data), type_(msg_data->type_) {}
Arg::Arg(const SharedString &name)
    : msg_data(std::make_shared<message::Arg>()), type_(ValType::none_) {
    msg_data->name_ = name;
}

std::shared_ptr<message::Arg> Arg::initMsg() {
    if (!this->msg_data) {
        this->msg_data = std::make_shared<message::Arg>();
        this->msg_data->type_ = this->type_;
    }
    return this->msg_data;
}

void Arg::mergeConfig(const Arg &other) {
    if (other.type_ != ValType::none_) {
        this->type_ = other.type_;
    }
    if (other.msg_data) {
        if (!this->msg_data) {
            this->msg_data = other.msg_data;
        } else {
            if (!other.msg_data->name_.empty()) {
                this->msg_data->name_ = other.msg_data->name_;
            }
            if (other.msg_data->init_) {
                init(*other.msg_data->init_);
            }
            if (other.msg_data->min_) {
                min(*other.msg_data->min_);
            }
            if (other.msg_data->max_) {
                max(*other.msg_data->max_);
            }
            if (other.msg_data->option_.size() > 0) {
                option(other.msg_data->option_);
            }
        }
    }
}

const std::string &Arg::name() const {
    return this->msg_data ? this->msg_data->name_.decode()
                          : SharedString::emptyStr();
}
const std::wstring &Arg::nameW() const {
    return this->msg_data ? this->msg_data->name_.decodeW()
                          : SharedString::emptyStrW();
}
ValType Arg::type() const { return this->type_; }
Arg &Arg::type(ValType type) {
    this->type_ = type;
    return *this;
}
std::optional<ValAdaptor> Arg::init() const {
    return this->msg_data ? this->msg_data->init_ : std::nullopt;
}
Arg &Arg::init(const ValAdaptor &init) {
    initMsg()->init_ = init;
    return *this;
}
std::optional<double> Arg::min() const {
    return this->msg_data ? this->msg_data->min_ : std::nullopt;
}
Arg &Arg::min(double min) {
    initMsg()->min_ = min;
    initMsg()->option_.clear();
    return *this;
}
std::optional<double> Arg::max() const {
    return this->msg_data ? this->msg_data->max_ : std::nullopt;
}
Arg &Arg::max(double max) {
    initMsg()->max_ = max;
    initMsg()->option_.clear();
    return *this;
}
const std::vector<ValAdaptor> &Arg::option() const {
    static std::vector<ValAdaptor> empty;
    return this->msg_data ? this->msg_data->option_ : empty;
}
Arg &Arg::option(std::vector<ValAdaptor> option) {
    initMsg()->option_ = std::move(option);
    initMsg()->min_ = std::nullopt;
    initMsg()->max_ = std::nullopt;
    return *this;
}

std::ostream &operator<<(std::ostream &os, const Arg &arg) {
    os << arg.name() << "(type=" << arg.type();
    auto min_ = arg.min();
    if (min_) {
        os << ", min=" << *min_;
    }
    auto max_ = arg.max();
    if (max_) {
        os << ", max=" << *max_;
    }
    if (arg.option().size() > 0) {
        os << ", option={";
        for (std::size_t j = 0; j < arg.option().size(); j++) {
            if (j > 0) {
                os << ", ";
            }
            os << static_cast<std::string>(arg.option()[j]);
        }
        os << "}";
    }
    os << ")";
    return os;
}

internal::FuncInfo::FuncInfo(const message::FuncInfo &m)
    : return_type(m.return_type), args(), func_impl(nullptr) {
    args.emplace();
    args->reserve(m.args.size());
    for (const auto &a : m.args) {
        args->emplace_back(a);
    }
}
message::FuncInfo internal::FuncInfo::toMessage(const SharedString &field) {
    message::FuncInfo m{0, field, return_type, {}};
    if (args.has_value()) {
        m.args.reserve(args->size());
        for (auto &a : *args) {
            m.args.emplace_back(a.initMsg());
        }
    }
    return m;
}

WEBCFACE_NS_END
