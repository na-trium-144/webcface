#include "webcface/func.h"
#include <thread>
#include <stdexcept>
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

Func::Func(const Field &base) : Field(base) {}
Func &Func::setImpl(const std::shared_ptr<FuncInfo> &v) {
    setCheck()->func_store.setSend(*this, v);
    return *this;
}
Func &Func::free() {
    dataLock()->func_store.unsetRecv(*this);
    return *this;
}

Func &Func::set(const std::vector<Arg> &args, ValType return_type,
                std::function<void(FuncCallHandle)> callback) {
    return setImpl(std::make_shared<FuncInfo>(
        return_type, args, true,
        [args_size = args.size(), callback = std::move(callback)](
            const std::vector<ValAdaptor> &args_vec) {
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
                // ignore exception
            }
            return result_f;
        }));
}
Func &Func::setAsync(const std::vector<Arg> &args, ValType return_type,
                     std::function<void(FuncCallHandle)> callback) {
    return setImpl(std::make_shared<FuncInfo>(
        return_type, args, true,
        [args_size = args.size(),
         callback = std::make_shared<std::function<void(FuncCallHandle)>>(
             std::move(callback))](std::vector<ValAdaptor> args_vec) {
            if (args_size != args_vec.size()) {
                throw std::invalid_argument(
                    "requires " + std::to_string(args_size) +
                    " arguments, got " + std::to_string(args_vec.size()));
            }
            return std::async(
                std::launch::async, [callback, args_vec = std::move(args_vec)] {
                    std::promise<ValAdaptor> result;
                    std::future<ValAdaptor> result_f = result.get_future();
                    FuncCallHandle handle{args_vec, std::move(result)};
                    callback->operator()(handle);
                    try {
                        handle.respond();
                    } catch (const std::future_error &) {
                        // ignore exception
                    }
                    return result_f.get();
                });
        }));
}

std::shared_future<ValAdaptor>
FuncInfo::run(const std::vector<ValAdaptor> &call_args, bool caller_async,
              const std::shared_ptr<internal::AsyncFuncState> &state) {
    std::shared_future<ValAdaptor> ret = func_impl(call_args).share();
    if (state) {
        state->setResultFuture(ret);
    }
    if (eval_async && caller_async) {
        std::thread([ret, state]() {
            ret.wait();
            if (state) {
                state->callResultEvent();
            }
        }).detach();
    } else {
        ret.wait();
        if (state) {
            state->callResultEvent();
        }
    }
    return ret;
}

/// \private
template <typename F1, typename F2, typename F3>
static void tryRun(F1 &&f_run, F2 &&f_ok, F3 &&f_fail) {
    ValAdaptor error;
    try {
        f_ok(f_run());
        return;
    } catch (const std::exception &e) {
        error = e.what();
    } catch (const std::string &e) {
        error = e;
    } catch (const char *e) {
        error = e;
    } catch (const std::wstring &e) {
        error = e;
    } catch (const wchar_t *e) {
        error = e;
    } catch (...) {
        error = "unknown exception";
    }
    f_fail(error);
}
void FuncInfo::run(webcface::message::Call &&call,
                   const std::shared_ptr<internal::ClientData> &data) {
    tryRun(
        [this, &call] {
            // まだこのスレッド内
            return func_impl(call.args);
        },
        [this, &call, &data](std::future<ValAdaptor> &&result_f) {
            if (eval_async) {
                std::weak_ptr<internal::ClientData> data_w = data;
                std::thread(
                    [data_w, call](std::future<ValAdaptor> result_f) {
                        // ここで別スレッドになるのでコピーキャプチャ
                        tryRun(
                            [&result_f] {
                                return result_f.get();
                                // 時間かかってこの間にdata消えるかもしれない
                            },
                            [&data_w, &call](const ValAdaptor &result) {
                                // ok
                                auto data = data_w.lock();
                                if (data) {
                                    data->message_push(
                                        webcface::message::packSingle(
                                            webcface::message::CallResult{
                                                {},
                                                call.caller_id,
                                                call.caller_member_id,
                                                false,
                                                result}));
                                }
                            },
                            [&data_w, &call](const ValAdaptor &error) {
                                // error
                                auto data = data_w.lock();
                                if (data) {
                                    data->message_push(
                                        webcface::message::packSingle(
                                            webcface::message::CallResult{
                                                {},
                                                call.caller_id,
                                                call.caller_member_id,
                                                true,
                                                error}));
                                }
                            });
                    },
                    std::move(result_f))
                    .detach();
            } else {
                // ok
                data->message_push(webcface::message::packSingle(
                    webcface::message::CallResult{{},
                                                  call.caller_id,
                                                  call.caller_member_id,
                                                  false,
                                                  result_f.get()}));
            }
        },
        [&call, &data](const ValAdaptor &error) {
            // まだこのスレッド内
            // error
            data->message_push(
                webcface::message::packSingle(webcface::message::CallResult{
                    {}, call.caller_id, call.caller_member_id, true, error}));
        });
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
        // selfの場合、新しいAsyncFuncResultに実行した結果を入れる
        auto func_info = data->func_store.getRecv(*this);
        auto state = std::make_shared<internal::AsyncFuncState>(
            static_cast<Field>(*this));
        if (func_info) {
            state->setStarted(true);
            try {
                (*func_info)->run(args_vec, true, state);
            } catch (...) {
                state->setResultException(std::current_exception());
            }
        } else {
            state->setStarted(false);
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
        target.setImpl(std::make_shared<FuncInfo>(**func_info));
        this->free();
        func_setter = nullptr;
        base_init = false;
    }
}

WEBCFACE_NS_END
