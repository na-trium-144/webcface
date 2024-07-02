#include <webcface/func.h>
#include <thread>
#include <chrono>
#include <stdexcept>
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"
#include <webcface/common/def.h>
#include "webcface/internal/event_target_impl.h"

WEBCFACE_NS_BEGIN

template class WEBCFACE_DLL_INSTANCE_DEF EventTarget<Func>;

Func::Func(const Field &base) : Field(base) {}
Func &Func::set3(const std::shared_ptr<FuncInfo> &v) {
    setCheck()->func_store.setSend(*this, v);
    return *this;
}
Func &Func::free() {
    dataLock()->func_store.unsetRecv(*this);
    return *this;
}

Func &Func::set(const std::vector<Arg> &args, ValType return_type, bool async,
                const std::function<void(FuncCallHandle)> &callback) {
    return set3(std::make_shared<FuncInfo>(
        return_type, args, async,
        [args_size = args.size(),
         callback](const std::vector<ValAdaptor> &args_vec) {
            if (args_size != args_vec.size()) {
                throw std::invalid_argument(
                    "requires " + std::to_string(args_size) +
                    " arguments, got " + std::to_string(args_vec.size()));
            }
            std::promise<ValAdaptor> result;
            std::future<ValAdaptor> result_f = result.get_future();
            FuncCallHandle handle{args_vec, std::move(result)};
            callback(handle);
            try {
                handle.respond();
            } catch (const std::future_error &) {
            }
            return result_f.get();
        }));
}

std::future<ValAdaptor> FuncInfo::run(const std::vector<ValAdaptor> &args,
                                      bool caller_async) {
    auto ret = func_impl(args);
    if (eval_async && caller_async) {
        return std::async(
            std::launch::async,
            [](std::future<ValAdaptor> &&ret) { return ret.get(); },
            std::move(ret));
    } else {
        ret.wait();
        return ret;
    }
}

ValAdaptor Func::run(const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    if (data->isSelf(*this)) {
        // selfの場合このスレッドでそのまま関数を実行する
        auto func_info = data->func_store.getRecv(*this);
        if (func_info) {
            return (*func_info)->run(args_vec, false).get();
        } else {
            throw FuncNotFound(*this);
        }
    } else {
        // リモートの場合runAsyncし結果が返るまで待機
        return this->runAsync(args_vec).result.get();
        // 例外が発生した場合はthrowされる
    }
}
AsyncFuncResult Func::runAsync(const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    if (data->isSelf(*this)) {
        // selfの場合、新しいAsyncFuncResultに別スレッドで実行した結果を入れる
        auto func_info = data->func_store.getRecv(*this);
        if (func_info) {
            return Internal::AsyncFuncState::running(
                       *this, (*func_info)->run(args_vec, true).share())
                ->getter();
        } else {
            return Internal::AsyncFuncState::notFound(*this)->getter();
        }
    } else {
        // リモートの場合cli.sync()を待たずに呼び出しメッセージを送る
        auto state = data->func_result_store.addResult(*this);
        data->message_push(Message::packSingle(Message::Call{
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
    return SharedString(Encoding::castToU8("..tmp" + std::to_string(id++)));
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
    if (!func_info) [[unlikely]] {
        throw std::runtime_error("AnonymousFunc not set");
    } else {
        target.set(std::make_shared<FuncInfo>(*func_info));
        this->free();
        func_setter = nullptr;
        base_init = false;
    }
}

WEBCFACE_NS_END
