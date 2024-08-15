#include "webcface/func.h"
#include <thread>
#include <stdexcept>
#include "webcface/internal/func_internal.h"
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

Func::Func(const Field &base) : Field(base) {}
Func &Func::setImpl(ValType return_type, std::vector<Arg> &&args,
                    std::function<FuncType> &&func_impl) {
    return setImpl(std::make_shared<internal::FuncInfo>(
        static_cast<Field>(*this), return_type, std::move(args),
        std::move(func_impl)));
}
Func &Func::setImpl(const std::shared_ptr<internal::FuncInfo> &v) {
    setCheck()->func_store.setSend(*this, v);
    return *this;
}
Func &Func::free() {
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
                data->message_push(webcface::message::packSingle(
                    webcface::message::CallResult{{},
                                                  call.caller_id,
                                                  call.caller_member_id,
                                                  true,
                                                  ValAdaptor(p.rejection())}));
            } else {
                data->message_push(webcface::message::packSingle(
                    webcface::message::CallResult{{},
                                                  call.caller_id,
                                                  call.caller_member_id,
                                                  false,
                                                  p.response()}));
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
            (*func_info)->run(state->setter());
        } else {
            state->setter().reach(false);
        }
        return state->getter();
    } else {
        // リモートの場合cli.sync()を待たずに呼び出しメッセージを送る
        auto state = data->func_result_store.addResult(*this);
        data->message_push(message::packSingle(message::Call{
            state->callerId(), 0, data->getMemberIdFromName(member_), field_,
            args_vec}));
        // resultはcli.onRecv内でセットされる。
        return state->getter();
    }
}

ValType Func::returnType() const {
    auto func_info = dataLock()->func_store.getRecv(*this);
    if (func_info) {
        return (*func_info)->return_type;
    }
    return ValType::none_;
}
std::vector<Arg> Func::args() const {
    auto func_info = dataLock()->func_store.getRecv(*this);
    if (func_info) {
        return (*func_info)->args;
    }
    return std::vector<Arg>{};
}

Func &Func::setArgs(const std::vector<Arg> &args) {
    auto func_info = setCheck()->func_store.getRecv(*this);
    if (!func_info) {
        throw std::invalid_argument("setArgs failed: Func not set");
    } else {
        if ((*func_info)->args.size() != args.size()) {
            throw std::invalid_argument(
                "setArgs failed: Number of args does not match, size: " +
                std::to_string(args.size()) +
                " actual: " + std::to_string((*func_info)->args.size()));
        }
        for (std::size_t i = 0; i < args.size(); i++) {
            (*func_info)->args[i].mergeConfig(args[i]);
        }
        return *this;
    }
}

SharedString AnonymousFunc::fieldNameTmp() {
    static int id = 0;
    return SharedString::fromU8String("..tmp" + std::to_string(id++));
}
AnonymousFunc &AnonymousFunc::operator=(AnonymousFunc &&other) noexcept {
    this->func_setter = std::move(other.func_setter);
    this->base_init = other.base_init;
    this->Func::operator=(std::move(static_cast<Func &>(other)));
    other.base_init = false;
    return *this;
}
void AnonymousFunc::lockTo(Func &target) {
    if (!base_init) {
        if (!func_setter) {
            throw std::runtime_error("Cannot lock empty AnonymousFunc");
        }
        this->data_w = target.data_w;
        this->member_ = target.member_;
        this->field_ = fieldNameTmp();
        func_setter(*this);
    }
    auto func_info = dataLock()->func_store.getRecv(*this);
    if (!func_info) {
        throw std::runtime_error("AnonymousFunc not set");
    } else {
        target.setImpl(std::make_shared<internal::FuncInfo>(**func_info));
        this->free();
        func_setter = nullptr;
        base_init = false;
    }
}

WEBCFACE_NS_END
