#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

void FuncStore::setFunc(const std::string &name, FuncType data) {
    std::lock_guard lock(mtx);
    funcs[name] = data;
}
FuncStore::FuncType FuncStore::getFunc(const std::string &name) {
    std::lock_guard lock(mtx);
    return funcs[name];
}
bool FuncStore::hasFunc(const std::string &name) {
    std::lock_guard lock(mtx);
    return funcs.find(name) != funcs.end();
}
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
    // 内部の呼び出しはAnyArgで、外部の呼び出しはstd::stringである めんどくさ
    if (from == "") {
        auto pr = std::make_shared<std::promise<FuncResult>>();
        auto &r = func_impl_store->addResult("", pr);
        r.from = "";
        r.name = name;
        if (func_impl_store->hasFunc(name)) {
            r.found = true;
            try {
                r.result = func_impl_store->getFunc(name).operator()(args_vec);
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
        std::vector<std::string> args(args_vec.size());
        for (std::size_t i = 0; i < args_vec.size(); i++) {
            args[i] = static_cast<std::string>(args_vec[i]);
        }
        return run_impl(args);
    }
}
std::future<FuncResult>
Func::run_impl(const std::vector<std::string> &args_vec) const {
    if (from == "") {
        std::vector<AnyArg> args(args_vec.size());
        for (std::size_t i = 0; i < args_vec.size(); i++) {
            args[i] = static_cast<AnyArg>(args_vec[i]);
        }
        return run_impl(args);
    } else {
        auto pr = std::make_shared<std::promise<FuncResult>>();
        auto &r = func_impl_store->addResult("", pr);
        r.from = this->from;
        r.name = this->name;
        this->cli->ws->getConnection()->send(Message::pack(
            Message::Call{{}, r.caller_id, "", r.from, r.name, args_vec}));
        return pr->get_future();
        // resultはclient.cc内でセットされる。
    }
}


} // namespace WebCFace