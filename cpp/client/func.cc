#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

FuncResult &
FuncResultStore::addResult(const std::string &caller,
                           std::shared_ptr<std::promise<FuncResult>> pr,
                           const std::string &member, const std::string &name) {
    std::lock_guard lock(mtx);
    int caller_id = results.size();
    results.push_back(FuncResult{caller_id, caller, pr, member, name});
    return results.back();
}
FuncResult &FuncResultStore::getResult(int caller_id) {
    std::lock_guard lock(mtx);
    return results.at(caller_id);
}

std::future<FuncResult>
Func::run_impl(const std::vector<ValAdaptor> &args_vec) const {
    auto pr = std::make_shared<std::promise<FuncResult>>();
    auto &r = cli->func_result_store.addResult("", pr, member_, name_);
    if (member_ == "") {
        auto func_info = store->getRecv("", name_);
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
    } else {
        // cliがセグフォする可能性
        this->cli->send(Message::pack(
            Message::Call{{}, r.caller_id, "", member_, name_, args_vec}));
        // resultはclient.cc内でセットされる。
    }
    return pr->get_future();
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