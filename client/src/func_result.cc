#include "webcface/func_result.h"
#include "webcface/internal/func_internal.h"
#include "webcface/common/internal/unlock.h"


WEBCFACE_NS_BEGIN

Promise internal::PromiseData::getter() {
    std::lock_guard lock(m);
    return Promise(base, shared_from_this(), started_f, result_f);
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

Promise::Promise(const Field &base,
                 const std::shared_ptr<internal::PromiseData> &data,
                 const std::shared_future<bool> &started,
                 const std::shared_future<ValAdaptor> &result)
    : Field(base), data(data), started(started), result(result) {}

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

CallHandle internal::PromiseData::setter() {
    std::lock_guard lock(m);
    return CallHandle(base, shared_from_this());
}
CallHandle::CallHandle(const Field &base,
                       const std::shared_ptr<internal::PromiseData> &data)
    : Field(base), data(data) {}


void CallHandle::reach(bool found) const {
    if (data) {
        std::lock_guard lock(data->m);
        if (!data->reached) {
            data->reached = true;
            data->found = found;
            data->started_p.set_value(found);
            data->callReachEvent();
            if (!found) {
                data->finished = true;
                data->is_error = true;
                data->rejection =
                    SharedString::encode(FuncNotFound(*this).what());
                data->result_p.set_exception(
                    std::make_exception_ptr(FuncNotFound(*this)));
                data->callFinishEvent();
            }
            data->cond.notify_all();
        } else {
            throw PromiseError("CallHandle::reach called twice");
        }
    } else {
        throw invalidHandle();
    }
}

bool CallHandle::respondable() const {
    if (data) {
        std::lock_guard lock(data->m);
        return !data->finished;
    } else {
        return false;
    }
}
void CallHandle::respond(const ValAdaptor &value) const {
    if (data) {
        if (respondable()) {
            std::lock_guard lock(data->m);
            data->finished = true;
            data->is_error = false;
            data->response = value;
            data->result_p.set_value(value);
            data->callFinishEvent();
            data->cond.notify_all();
        } else {
            throw PromiseError(
                "already responded or rejected with this CallHandle");
        }
    } else {
        throw invalidHandle();
    }
}
void CallHandle::reject(const ValAdaptor &message) const {
    if (data) {
        if (respondable()) {
            std::lock_guard lock(data->m);
            data->finished = true;
            data->is_error = true;
            data->rejection = message;
            data->result_p.set_exception(std::make_exception_ptr(
                Rejection(data->base, std::string(message.asStringView()))));
            data->callFinishEvent();
            data->cond.notify_all();
        } else {
            throw PromiseError(
                "already responded or rejected with this CallHandle");
        }
    } else {
        throw invalidHandle();
    }
}
bool CallHandle::assertArgsNum(std::size_t expected) const {
    if (data) {
        if (args().size() == expected) {
            return true;
        } else {
            reject(strJoin(name(), "() requires ", std::to_string(expected),
                           " arguments, but received ",
                           std::to_string(args().size())));
            return false;
        }
    } else {
        throw invalidHandle();
    }
}

Promise &Promise::onReach(std::function<void(Promise)> callback) {
    if (data) {
        std::lock_guard lock(data->m);
        if (!data->reach_event_done) {
            data->reach_event = std::move(callback);
            data->callReachEvent();
        }
        return *this;
    } else {
        throw invalidPromise();
    }
}
Promise &Promise::onFinish(std::function<void(Promise)> callback) {
    if (data) {
        std::lock_guard lock(data->m);
        if (!data->finish_event_done) {
            data->finish_event = std::move(callback);
            data->callFinishEvent();
        }
        return *this;
    } else {
        throw invalidPromise();
    }
}
void internal::PromiseData::callReachEvent() {
    std::function<void(Promise)> event;
    if (!reach_event_done && reached && reach_event) {
        reach_event_done = true;
        event = std::move(reach_event);
    }
    if (event) {
        ScopedUnlock unlock(m);
        event(getter());
    }
}
void internal::PromiseData::callFinishEvent() {
    std::function<void(Promise)> event;
    if (!finish_event_done && finished && finish_event) {
        finish_event_done = true;
        event = std::move(finish_event);
    }
    if (event) {
        ScopedUnlock unlock(m);
        event(getter());
    }
}

void Promise::waitReachImpl(
    std::optional<std::chrono::microseconds> timeout) const {
    if (data) {
        std::unique_lock lock(data->m);
        if (timeout) {
            data->cond.wait_for(lock, *timeout, [&] { return data->reached; });
        } else {
            data->cond.wait(lock, [&] { return data->reached; });
        }
    } else {
        throw invalidPromise();
    }
}
void Promise::waitFinishImpl(
    std::optional<std::chrono::microseconds> timeout) const {
    if (data) {
        std::unique_lock lock(data->m);
        if (timeout) {
            data->cond.wait_for(lock, *timeout, [&] { return data->finished; });
        } else {
            data->cond.wait(lock, [&] { return data->finished; });
        }
    } else {
        throw invalidPromise();
    }
}

bool Promise::reached() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->reached;
    } else {
        throw invalidPromise();
    }
}
bool Promise::found() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->found;
    } else {
        throw invalidPromise();
    }
}
bool Promise::finished() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->finished;
    } else {
        throw invalidPromise();
    }
}
bool Promise::isError() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->is_error;
    } else {
        throw invalidPromise();
    }
}
ValAdaptor Promise::response() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->response;
    } else {
        throw invalidPromise();
    }
}
StringView Promise::rejection() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->rejection.asStringView();
    } else {
        throw invalidPromise();
    }
}
WStringView Promise::rejectionW() const {
    if (data) {
        std::lock_guard lock(data->m);
        return data->rejection.asWStringView();
    } else {
        throw invalidPromise();
    }
}

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

SanityError Promise::invalidPromise() {
    return SanityError("Promise does not have valid "
                       "pointer to function call");
}

SanityError CallHandle::invalidHandle() {
    return SanityError("CallHandle does not have valid "
                       "pointer to function call");
}

template <std::size_t v_index, typename CVal>
std::vector<CVal> &internal::PromiseData::initCArgs() {
    if (this->c_args_.index() != v_index) {
        std::vector<CVal> c_args;
        c_args.reserve(this->args_.size());
        for (const auto &a : this->args_) {
            CVal cv;
            cv.as_int = a.as<int>();
            cv.as_double = a.as<double>();
            if constexpr (v_index == 1) {
                cv.as_str = static_cast<std::string_view>(a).data();
            } else {
                cv.as_str = static_cast<std::wstring_view>(a).data();
            }
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
const std::vector<ValAdaptorVector> &CallHandle::args() const {
    if (data) {
        return data->args_;
    } else {
        throw invalidHandle();
    }
}

WEBCFACE_NS_END
