#include <webcface/func_info.h>
#include "webcface/message/message.h"

WEBCFACE_NS_BEGIN

void Arg::mergeConfig(const Arg &rhs) {
    if (!rhs.name_.empty()) {
        name_ = rhs.name_;
    }
    if (rhs.type_ != ValType::none_) {
        type_ = rhs.type_;
    }
    if (rhs.init_) {
        init(*rhs.init_);
    }
    if (rhs.min_) {
        min(*rhs.min_);
    }
    if (rhs.max_) {
        max(*rhs.max_);
    }
    if (rhs.option_.size() > 0) {
        option(rhs.option_);
    }
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

Arg::Arg(const message::Arg &a)
    : name_(a.name_), type_(a.type_), init_(a.init_), min_(a.min_),
      max_(a.max_), option_(a.option_) {}
message::Arg Arg::toMessage() const {
    return message::Arg(name_, type_, init_, min_, max_, option_);
}

FuncInfo::FuncInfo(const message::FuncInfo &m)
    : return_type(m.return_type), args(), func_impl(nullptr) {
    args.reserve(m.args->size());
    for (const auto &a : *m.args) {
        args.emplace_back(a);
    }
}
message::FuncInfo FuncInfo::toMessage(const SharedString &field) const {
    message::FuncInfo m{0, field, return_type,
                        std::make_shared<std::vector<message::Arg>>()};
    m.args->reserve(args.size());
    for (const auto &a : args) {
        m.args->emplace_back(a.toMessage());
    }
    return m;
}

WEBCFACE_NS_END
