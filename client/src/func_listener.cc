#include <webcface/func.h>
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
FuncListener::FuncListener(const Field &base) : Func(base) {}

FuncListener &FuncListener::listen() {
    this->Func::setRaw({
        this->return_type_,
        this->args_,
        [*this](const std::vector<ValAdaptor> &args_vec) {
            if (this->args_.size() != args_vec.size()) {
                throw std::invalid_argument(
                    "requires " + std::to_string(this->args_.size()) +
                    " arguments, got " + std::to_string(args_vec.size()));
            }
            std::promise<ValAdaptor> result;
            std::future<ValAdaptor> result_f = result.get_future();
            this->dataLock()->func_listener_handlers[this->field_].push(
                FuncCallHandle{args_vec, std::move(result)});
            return result_f.get();
        },
        nullptr,
    });
    return *this;
}

std::optional<FuncCallHandle> FuncListener::fetchCall() const {
    return this->setCheck()->func_listener_handlers[this->field_].pop();
}

WEBCFACE_NS_END
