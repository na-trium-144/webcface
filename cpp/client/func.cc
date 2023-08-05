#include <webcface/webcface.h>
#include <thread>
#include <chrono>
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
            os << r.result.get();
        } catch (const std::exception &e) {
            os << "<Error> " << e.what();
        }
    }
    return os;
}

ValAdaptor Func::run(const std::vector<ValAdaptor> &args_vec) const {
    if (member_ == "") {
        // selfの場合このスレッドでそのまま関数を実行する
        auto func_info = dataLock()->func_store.getRecv("", name_);
        if (func_info) {
            return func_info->func_impl(args_vec);
        } else {
            throw FuncNotFound(member_, name_);
        }
    } else {
        // リモートの場合runAsyncし結果が返るまで待機
        auto &async_res = this->runAsync(args_vec);
        return async_res.result.get();
        // 例外が発生した場合はthrowされる
    }
}
AsyncFuncResult &Func::runAsync(const std::vector<ValAdaptor> &args_vec) const {
    auto data_s = dataLock();
    auto &r = data_s->func_result_store.addResult(data, "", member_, name_);
    if (member_ == "") {
        // selfの場合、新しいAsyncFuncResultに別スレッドで実行した結果を入れる
        std::thread([data_s, member_ = this->member_, name_ = this->name_, args_vec, r] {
            auto func_info = data_s->func_store.getRecv("", name_);
            if (func_info) {
                r.started_->set_value(true);
                try {
                    auto ret = func_info->func_impl(args_vec);
                    r.result_->set_value(ret);
                } catch (...) {
                    r.result_->set_exception(std::current_exception());
                }
            } else {
                r.started_->set_value(false);
                try {
                    throw FuncNotFound(member_, name_);
                } catch (...) {
                    r.result_->set_exception(std::current_exception());
                }
            }
        }).detach();
    } else {
        // リモートの場合cli.sync()を待たずに呼び出しメッセージを送る
        data_s->func_call_queue.push(
            {r.caller_id, "", member_, name_, args_vec});
        // resultはcli.onRecv内でセットされる。
    }
    return r;
}

ValType Func::returnType() {
    auto func_info = dataLock()->func_store.getRecv(member_, name_);
    if (func_info) {
        return func_info->return_type;
    }
    return ValType::none_;
}
std::vector<Arg> Func::args() {
    auto func_info = dataLock()->func_store.getRecv(member_, name_);
    if (func_info) {
        return func_info->args;
    }
    return std::vector<Arg>{};
}
Func &Func::setArgs(const std::vector<Arg> &args) {
    assert(member_ == "" && "Cannot set data to member other than self");
    auto func_info = dataLock()->func_store.getRecv(member_, name_);
    assert(func_info != std::nullopt && "Func not set");
    assert(func_info->args.size() == args.size() &&
           "Number of args does not match");
    for (std::size_t i = 0; i < args.size(); i++) {
        func_info->args[i].mergeConfig(args[i]);
    }
    dataLock()->func_store.setSend(name_, *func_info);
    return *this;
}


} // namespace WebCFace