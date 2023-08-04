#include <webcface/webcface.h>
#include <thread>
#include <chrono>
#include "../message/message.h"

namespace WebCFace {
auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &data) {
    os << "Func(\"" << data.name() << "\"): ";
    if (data.started.wait_for(std::chrono::seconds(0)) !=
        std::future_status::ready) {
        os << "<Connecting>";
    } else if (data.started.get() == false) {
        os << "<Not Found>";
    } else if (data.result.wait_for(std::chrono::seconds(0)) !=
               std::future_status::ready) {
        os << "<Running>";
    } else {
        try {
            os << data.result.get();
        } catch (const std::exception &e) {
            os << "<Error> " << e.what();
        }
    }
    return os;
}
Member AsyncFuncResult::member() const { return cli->member(member_); }

AsyncFuncResult &FuncResultStore::addResult(const std::string &caller,
                                            const std::string &member,
                                            const std::string &name) {
    std::lock_guard lock(mtx);
    int caller_id = results.size();
    results.push_back(AsyncFuncResult{caller_id, cli, caller, member, name});
    return results.back();
}
AsyncFuncResult &FuncResultStore::getResult(int caller_id) {
    std::lock_guard lock(mtx);
    return results.at(caller_id);
}

ValAdaptor Func::run(const std::vector<ValAdaptor> &args_vec) const {
    if (member_ == "") {
        auto func_info = store->getRecv("", name_);
        if (func_info) {
            return func_info->func_impl(args_vec);
        } else {
            throw FuncNotFound(member_, name_);
        }
    } else {
        auto &async_res = this->runAsync(args_vec);
        return async_res.result.get();
        // 例外が発生した場合はthrowされる
    }
}
AsyncFuncResult &Func::runAsync(const std::vector<ValAdaptor> &args_vec) const {
    auto &r = cli->func_result_store.addResult("", member_, name_);
    if (member_ == "") {
        std::thread([cli = this->cli, name_ = this->name_, args_vec, &r] {
            try {
                auto ret = cli->self().func(name_).run(args_vec);
                r.started_->set_value(true);
                r.result_->set_value(ret);
            } catch (const FuncNotFound &e) {
                r.started_->set_value(false);
                r.result_->set_exception(std::current_exception());
            } catch (...) {
                r.started_->set_value(true);
                r.result_->set_exception(std::current_exception());
            }
        }).detach();
    } else {
        // cliがセグフォする可能性
        this->cli->send(Message::pack(
            Message::Call{{}, r.caller_id, "", member_, name_, args_vec}));
        // resultはclient.cc内でセットされる。
    }
    return r;
}

ValType Func::returnType() {
    auto func_info = store->getRecv(member_, name_);
    if (func_info) {
        return func_info->return_type;
    }
    return ValType::none_;
}
std::vector<ValType> Func::argsType() {
    auto func_info = store->getRecv(member_, name_);
    if (func_info) {
        return func_info->args_type;
    }
    return std::vector<ValType>{};
}


} // namespace WebCFace