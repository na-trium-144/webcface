#include <webcface/func_listener.h>
#include <webcface/func.h>
#include "client_internal.h"

namespace webcface {
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
            auto result = std::make_shared<std::promise<ValAdaptor>>();
            this->dataLock()->func_listener_handlers[this->field_].push(
                FuncListenerHandler{args_vec, result});
            return result->get_future().get();
        },
        nullptr,
        this->hidden_,
    });
    return *this;
}

std::optional<FuncListenerHandler> FuncListener::fetchCall() const {
    return this->setCheck()->func_listener_handlers[this->field_].pop();
}

} // namespace webcface