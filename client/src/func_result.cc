#include "webcface/func_result.h"
#include "webcface/internal/func_internal.h"


WEBCFACE_NS_BEGIN

FuncNotFound::FuncNotFound(const FieldBase &base)
    : std::runtime_error("member(\"" + base.member_.decode() + "\")" +
                         ".func(\"" + base.field_.decode() + "\") is not set") {
}

void internal::PromiseData::reach(bool found) {
    std::lock_guard lock(m);
    this->reached = true;
    this->found = found;
    started_p.set_value(found);
    callReachEvent();
    if (!found) {
        this->finished = true;
        try {
            throw FuncNotFound(base);
        } catch (const FuncNotFound &e) {
            this->rejection = SharedString::encode(e.what());
            this->result_p.set_exception(std::current_exception());
        }
        callFinishEvent();
    }
}
void internal::PromiseData::respond(const ValAdaptor &result_val) {
    std::lock_guard lock(m);
    this->finished = true;
    this->response = result_val;
    this->result_p.set_value(result_val);
    callFinishEvent();
}
void CallHandle::respond(const ValAdaptor &value) const {
    if (data) {
        data->respond(value);
    } else {
        throw invalidHandle();
    }
}
void internal::PromiseData::reject(const ValAdaptor &message) {
    std::lock_guard lock(m);
    this->finished = true;
    this->rejection = message;
    try {
        throw std::runtime_error(message.asStringRef().c_str());
    } catch (...) {
        this->result_p.set_exception(std::current_exception());
    }
    callFinishEvent();
}
void CallHandle::reject(const ValAdaptor &message) const {
    if (data) {
        data->reject(message);
    } else {
        throw invalidHandle();
    }
}
void internal::PromiseData::setReachEvent(
    std::function<void(Promise)> &&callback) {
    std::lock_guard lock(m);
    reach_event = std::move(callback);
    callReachEvent();
}
Promise &Promise::onReached(std::function<void(Promise)> callback) {
    data->setReachEvent(std::move(callback));
    return *this;
}
void internal::PromiseData::setFinishEvent(
    std::function<void(Promise)> &&callback) {
    std::lock_guard lock(m);
    finish_event = std::move(callback);
    callFinishEvent();
}
Promise &Promise::onFinished(std::function<void(Promise)> callback) {
    data->setFinishEvent(std::move(callback));
    return *this;
}
void internal::PromiseData::callReachEvent() {
    std::lock_guard lock(m);
    if (!reach_event_done && reached && reach_event) {
        reach_event_done = true;
        reach_event(getter());
    }
}
void internal::PromiseData::callFinishEvent() {
    std::lock_guard lock(m);
    if (!finish_event_done && finished && finish_event) {
        finish_event_done = true;
        finish_event(getter());
    }
}

Promise internal::PromiseData::getter() {
    std::lock_guard lock(m);
    return Promise(base, shared_from_this(), started_f, result_f);
}
Promise::Promise(const Field &base,
                 const std::shared_ptr<internal::PromiseData> &data,
                 const std::shared_future<bool> &started,
                 const std::shared_future<ValAdaptor> &result)
    : Field(base), data(data), started(started), result(result) {}

CallHandle internal::PromiseData::setter() {
    std::lock_guard lock(m);
    return CallHandle(base, shared_from_this());
}
CallHandle::CallHandle(const Field &base,
                       const std::shared_ptr<internal::PromiseData> &data)
    : Field(base), data(data) {}

// std::ostream &operator<<(std::ostream &os, const Promise &r) {
//     os << "Func(\"" << r.name() << "\"): ";
//     if (r.started.wait_for(std::chrono::seconds(0)) !=
//         std::future_status::ready) {
//         os << "<Connecting>";
//     } else if (r.started.get() == false) {
//         os << "<Not Found>";
//     } else if (r.result.wait_for(std::chrono::seconds(0)) !=
//                std::future_status::ready) {
//         os << "<Running>";
//     } else {
//         try {
//             os << static_cast<std::string>(r.result.get());
//         } catch (const std::exception &e) {
//             os << "<Error> " << e.what();
//         }
//     }
//     return os;
// }

std::runtime_error &CallHandle::invalidHandle() {
    static std::runtime_error invalid_handle("CallHandle does not have valid "
                                             "pointer to function call");
    return invalid_handle;
}

template <std::size_t v_index, typename CVal>
std::vector<CVal> &internal::PromiseData::initCArgs() {
    if (this->c_args_.index() != v_index) {
        std::vector<CVal> c_args;
        c_args.reserve(this->args_.size());
        for (const auto &a : this->args_) {
            CVal cv;
            cv.as_int = a;
            cv.as_double = a;
            cv.as_str = a;
            c_args.push_back(cv);
        }
        this->c_args_.emplace<v_index>(std::move(c_args));
    }
    return std::get<v_index>(this->c_args_);
}
const wcfMultiVal *CallHandle::cArgs() const {
    if (data) {
        auto &c_args = data->initCArgs<1, wcfMultiVal>();
        return c_args.data();
    } else {
        throw invalidHandle();
    }
}
const wcfMultiValW *CallHandle::cWArgs() const {
    if (data) {
        auto &c_args = data->initCArgs<2, wcfMultiValW>();
        return c_args.data();
    } else {
        throw invalidHandle();
    }
}
const std::vector<ValAdaptor> &CallHandle::args() const {
    if (data) {
        return data->args_;
    } else {
        throw invalidHandle();
    }
}

WEBCFACE_NS_END
