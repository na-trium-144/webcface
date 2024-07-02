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
Func &Func::setRaw(const std::shared_ptr<FuncInfo> &v) {
    setCheck()->func_store.setSend(*this, v);
    return *this;
}
Func &Func::free() {
    dataLock()->func_store.unsetRecv(*this);
    return *this;
}

Func &Func::set(const std::vector<Arg> &args, ValType return_type,
                const std::function<void(FuncCallHandle)> &callback) {
    return setRaw({return_type, args,
                   [args_size = args.size(),
                    callback](const std::vector<ValAdaptor> &args_vec) {
                       if (args_size != args_vec.size()) {
                           throw std::invalid_argument(
                               "requires " + std::to_string(args_size) +
                               " arguments, got " +
                               std::to_string(args_vec.size()));
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
                   }});
}
// Func &Func::replaceImpl(FuncType func) {
//     auto func_info = setCheck()->func_store.getRecv(*this);
//     if (func_info == std::nullopt) {
//         throw std::invalid_argument("replaceImpl failed: Func not set");
//     }
//     (*func_info)->func_impl = func;
//     return *this;
// }
// FuncType Func::getImpl() const {
//     auto func_info = setCheck()->func_store.getRecv(*this);
//     if (func_info == std::nullopt) {
//         throw std::invalid_argument("getImpl failed: Func not set");
//     }
//     return (*func_info)->func_impl;
// }

void Func::runImpl(std::size_t caller_id,
                   const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    auto func_info = data->func_store.getRecv(*this);
    if (func_info) {
        data->func_result_store.resultSetter(caller_id).setStarted(true);
        try {
            auto ret = (*func_info)->run(args_vec);
            data->func_result_store.resultSetter(caller_id).setResult(ret);
        } catch (...) {
            data->func_result_store.resultSetter(caller_id).setResultException(
                std::current_exception());
        }
    } else {
        data->func_result_store.resultSetter(caller_id).setStarted(false);
    }
    data->func_result_store.removeResultSetter(caller_id);
}
ValAdaptor Func::run(const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    if (data->isSelf(*this)) {
        auto r = data->func_result_store.addResult(*this);
        // selfの場合このスレッドでそのまま関数を実行する
        runImpl(r.caller_id, args_vec);
        return r.result.get();
    } else {
        // リモートの場合runAsyncし結果が返るまで待機
        auto async_res = this->runAsync(args_vec);
        return async_res.result.get();
        // 例外が発生した場合はthrowされる
    }
}
AsyncFuncResult Func::runAsync(const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    auto r = data->func_result_store.addResult(*this);
    if (data->isSelf(*this)) {
        // selfの場合、新しいAsyncFuncResultに別スレッドで実行した結果を入れる
        std::thread([*this, caller_id = r.caller_id, args_vec]() {
            runImpl(caller_id, args_vec);
        }).detach();
    } else {
        // リモートの場合cli.sync()を待たずに呼び出しメッセージを送る
        data->message_push(Message::packSingle(
            FuncCall{r.caller_id, 0, data->getMemberIdFromName(member_), field_,
                     args_vec}
                .toMessage()));
        // resultはcli.onRecv内でセットされる。
    }
    return r;
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
        target.setRaw(*func_info);
        this->free();
        func_setter = nullptr;
        base_init = false;
    }
}

WEBCFACE_NS_END
