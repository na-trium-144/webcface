#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

FuncResult &FuncStore::addResult(const std::string &caller,
                                 std::shared_ptr<std::promise<FuncResult>> pr) {
    std::lock_guard lock(mtx);
    int caller_id = results.size();
    results.push_back(FuncResult{caller_id, caller, pr});
    return results.back();
}
FuncResult &FuncStore::getResult(int caller_id) {
    std::lock_guard lock(mtx);
    return results.at(caller_id);
}

std::future<FuncResult>
Func::run_impl(const std::vector<AnyArg> &args_vec) const {
    if (from == "") {
        auto pr = std::make_shared<std::promise<FuncResult>>();
        auto &r = func_impl_store->addResult("", pr);
        r.from = "";
        r.name = name;
        auto func_info = store->getRecv("", name);
        if (func_info) {
            r.found = true;
            try {
                r.result = func_info->func_impl(args_vec);
            } catch (const std::exception &e) {
                r.error_msg = e.what();
                r.is_error = true;
            }
        } else {
            r.found = false;
        }
        r.setReady();
        return pr->get_future();
    } else {
        auto pr = std::make_shared<std::promise<FuncResult>>();
        auto &r = func_impl_store->addResult("", pr);
        r.from = this->from;
        r.name = this->name;
        // cliがセグフォする可能性
        this->cli->send(Message::pack(
            Message::Call{{}, r.caller_id, "", r.from, r.name, args_vec}));
        return pr->get_future();
        // resultはclient.cc内でセットされる。
    }
}

AbstArgType Func::returnType() const {
    auto func_info = store->getRecv(from, name);
    if (func_info) {
        return func_info->return_type;
    }
    return AbstArgType::none_;
}
std::vector<AbstArgType> Func::argsType() const {
    auto func_info = store->getRecv(from, name);
    if (func_info) {
        return func_info->args_type;
    }
    return std::vector<AbstArgType>{};
}


} // namespace WebCFace