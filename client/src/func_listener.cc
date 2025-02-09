#include "webcface/func.h"
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
FuncListener::FuncListener(const Field &base) : Func(base) {}

FuncListener &FuncListener::listen() {
    this->Func::setImpl(
        this->return_type_, std::vector(this->args_),
        [this_ = static_cast<Field>(*this),
         args_num = this->args_.size()](const CallHandle &handle) {
            if (handle.assertArgsNum(args_num)) {
                this_.dataLock()
                    ->func_listener_handlers.lock()
                    .get()[this_.field_]
                    .push(handle);
            }
        });
    return *this;
}

std::optional<FuncCallHandle> FuncListener::fetchCall() const {
    return this->setCheck()
        ->func_listener_handlers.lock()
        .get()[this->field_]
        .pop();
}

WEBCFACE_NS_END
