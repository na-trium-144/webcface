#include <webcface/func.h>
#include <thread>
#include <chrono>
#include <stdexcept>
#include "../message/message.h"

namespace WebCFace {

auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &r) {
    os << "Func(\"" << r.name() << "\"): ";
    if (r.started.wait_for(std::chrono::seconds(0)) !=
        std::future_status::ready) {
        os << "<Connecting>";
    } else if (r.started.get() == false) {
        os << "<Not Found>";
    } else if (r.result.wait_for(std::chrono::seconds(0)) !=
               std::future_status::ready) {
        os << "<Running>";
    } else {
        try {
            os << static_cast<std::string>(r.result.get());
        } catch (const std::exception &e) {
            os << "<Error> " << e.what();
        }
    }
    return os;
}

ValAdaptor Func::run(const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    if (data->isSelf(*this)) {
        // selfの場合このスレッドでそのまま関数を実行する
        auto func_info = data->func_store.getRecv(*this);
        if (func_info) {
            return func_info->run(args_vec);
        } else {
            throw FuncNotFound(*this);
        }
    } else {
        // リモートの場合runAsyncし結果が返るまで待機
        auto &async_res = this->runAsync(args_vec);
        return async_res.result.get();
        // 例外が発生した場合はthrowされる
    }
}
AsyncFuncResult &Func::runAsync(const std::vector<ValAdaptor> &args_vec) const {
    auto data = dataLock();
    auto &r = data->func_result_store.addResult("", *this);
    if (data->isSelf(*this)) {
        // selfの場合、新しいAsyncFuncResultに別スレッドで実行した結果を入れる
        std::thread([data, base = *this, args_vec, r] {
            auto func_info = data->func_store.getRecv(base);
            if (func_info) {
                r.started_->set_value(true);
                try {
                    auto ret = func_info->run(args_vec);
                    r.result_->set_value(ret);
                } catch (...) {
                    r.result_->set_exception(std::current_exception());
                }
            } else {
                r.started_->set_value(false);
                try {
                    throw FuncNotFound(base);
                } catch (...) {
                    r.result_->set_exception(std::current_exception());
                }
            }
        }).detach();
    } else {
        // リモートの場合cli.sync()を待たずに呼び出しメッセージを送る
        data->message_queue.push(Message::packSingle(Message::Call{
            FuncCall{r.caller_id, 0, data->getMemberIdFromName(member_), field_,
                     args_vec}}));
        // resultはcli.onRecv内でセットされる。
    }
    return r;
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
        return func_info->args;
    }
    return std::vector<Arg>{};
}
Func &Func::setArgs(const std::vector<Arg> &args) {
    auto data = dataLock();
    auto func_info = data->func_store.getRecv(*this);
    if (func_info == std::nullopt) {
        throw std::invalid_argument("setArgs failed: Func not set");
    }
    if (func_info->args.size() != args.size()) {
        throw std::invalid_argument(
            "setArgs failed: Number of args does not match, size: " +
            std::to_string(args.size()) +
            " actual: " + std::to_string(func_info->args.size()));
    }
    for (std::size_t i = 0; i < args.size(); i++) {
        func_info->args[i].mergeConfig(args[i]);
    }
    set(*func_info);
    return *this;
}

Func &Func::setRunCond(FuncWrapperType wrapper) {
    auto data = dataLock();
    auto func_info = data->func_store.getRecv(*this);
    assert(func_info != std::nullopt && "Func not set");
    func_info->func_wrapper = wrapper;
    set(*func_info);
    return *this;
}

FuncWrapperType
FuncWrapper::runCondOnSync(const std::weak_ptr<ClientData> &data) {
    return [data](FuncType callback, const std::vector<ValAdaptor> &args) {
        auto data_s = data.lock();
        if (data_s) {
            auto sync = std::make_shared<ClientData::FuncOnSync>();
            data_s->func_sync_queue.push(sync);
            struct ScopeGuard {
                std::shared_ptr<ClientData::FuncOnSync> sync;
                explicit ScopeGuard(
                    const std::shared_ptr<ClientData::FuncOnSync> &sync)
                    : sync(sync) {
                    sync->wait();
                }
                ~ScopeGuard() { sync->done(); }
            };
            {
                ScopeGuard scope_guard{sync};
                return callback(args);
            }
        }
        return ValAdaptor{};
    };
}

} // namespace WebCFace