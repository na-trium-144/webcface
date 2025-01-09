#include "webcface/common/internal/message/pack.h"
#include "webcface/func.h"
#include <stdexcept>
#include "webcface/common/internal/message/func.h"
#include "webcface/internal/func_internal.h"
#include "webcface/internal/client_internal.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

Func::Func(const Field &base) : Field(base) {}
const Func &Func::setImpl(ValType return_type, std::vector<Arg> &&args,
                          std::function<FuncType> &&func_impl) const {
    return setImpl(std::make_shared<internal::FuncInfo>(
        static_cast<Field>(*this), return_type, std::move(args),
        std::move(func_impl)));
}
const Func &Func::setImpl(ValType return_type, std::nullopt_t,
                          std::function<FuncType> &&func_impl) const {
    return setImpl(std::make_shared<internal::FuncInfo>(
        static_cast<Field>(*this), return_type, std::nullopt,
        std::move(func_impl)));
}
const Func &Func::setImpl(const std::shared_ptr<internal::FuncInfo> &v) const {
    setCheck()->func_store.setSend(*this, v);
    return *this;
}
const Func &Func::free() const {
    dataLock()->func_store.unsetRecv(*this);
    return *this;
}

void internal::FuncInfo::run(webcface::message::Call &&call) {
    auto state = std::make_shared<internal::PromiseData>(
        static_cast<Field>(*this), std::move(call.args));
    state->setter().reach(true);
    this->run(state->setter());
    state->getter().onFinish([call = std::move(call),
                              data_w = this->data_w](const Promise &p) {
        auto data = data_w.lock();
        if (data) {
            if (p.isError()) {
                data->messagePushAlways(
                    webcface::message::CallResult{{},
                                                  call.caller_id,
                                                  call.caller_member_id,
                                                  true,
                                                  ValAdaptor(p.rejection())});
            } else {
                data->messagePushAlways(
                    webcface::message::CallResult{{},
                                                  call.caller_id,
                                                  call.caller_member_id,
                                                  false,
                                                  p.response()});
            }
        }
    });
}

AsyncFuncResult Func::runAsync(std::vector<ValAdaptor> args_vec) const {
    auto data = dataLock();
    if (data->isSelf(*this)) {
        // selfの場合、新しいAsyncFuncResultに実行した結果を入れる
        auto func_info = data->func_store.getRecv(*this);
        auto state = std::make_shared<internal::PromiseData>(
            static_cast<Field>(*this), std::move(args_vec));
        if (func_info) {
            state->setter().reach(true);
            func_info->run(state->setter());
        } else {
            state->setter().reach(false);
        }
        return state->getter();
    } else {
        // リモートの場合cli.sync()を待たずに呼び出しメッセージを送る
        auto state = data->func_result_store.addResult(*this);
        if (!data->messagePushOnline(message::Call{
                state->callerId(), 0, data->getMemberIdFromName(member_),
                field_, args_vec})) {
            state->setter().reach(false);
        }
        // resultはcli.onRecv内でセットされる。
        return state->getter();
    }
}

ValType Func::returnType() const {
    auto func_info = dataLock()->func_store.getRecv(*this);
    if (func_info) {
        return func_info->return_type;
    }
    return ValType::none_;
}
std::vector<Arg> Func::args() const {
    auto func_info = dataLock()->func_store.getRecv(*this);
    if (func_info) {
        return func_info->args.value_or(std::vector<Arg>{});
    }
    return std::vector<Arg>{};
}
bool Func::exists() const {
    std::lock_guard lock(dataLock()->func_store.mtx);
    return dataLock()->func_store.getEntry(member_).count(field_);
}

const Func &Func::setArgs(const std::vector<Arg> &args) const {
    auto func_info = setCheck()->func_store.getRecv(*this);
    if (!func_info) {
        throw std::invalid_argument("setArgs failed: Func not set");
    } else {
        if (func_info->args.has_value()) {
            if (func_info->args->size() != args.size()) {
                throw std::invalid_argument(
                    "setArgs failed: Number of args does not match, size: " +
                    std::to_string(args.size()) +
                    " actual: " + std::to_string(func_info->args->size()));
            }
            for (std::size_t i = 0; i < args.size(); i++) {
                func_info->args->at(i).mergeConfig(args[i]);
            }
        } else {
            func_info->args.emplace(args);
        }
        return *this;
    }
}
const Func &Func::setReturnType(ValType return_type) const {
    auto func_info = setCheck()->func_store.getRecv(*this);
    if (!func_info) {
        throw std::invalid_argument("setReturnType failed: Func not set");
    } else {
        func_info->return_type = return_type;
        return *this;
    }
}

WEBCFACE_NS_END
