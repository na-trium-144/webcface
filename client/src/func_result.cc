#include <webcface/func_result.h>
#include <webcface/internal/func_internal.h>

WEBCFACE_NS_BEGIN

FuncNotFound::FuncNotFound(const FieldBase &base)
    : std::runtime_error("member(\"" + base.member_.decode() + "\")" +
                         ".func(\"" + base.field_.decode() + "\") is not set") {
}

eventpp::CallbackList<void(bool)> &AsyncFuncResult::onStarted() const {
    return state->startedEvent();
}
eventpp::CallbackList<void(std::shared_future<ValAdaptor>)> &
AsyncFuncResult::onResult() const {
    return state->resultEvent();
}

std::shared_ptr<internal::AsyncFuncState>
internal::AsyncFuncState::notFound(const Field &base) {
    auto state = std::make_shared<internal::AsyncFuncState>(base);
    state->setStarted(false);
    return state;
}
std::shared_ptr<internal::AsyncFuncState> internal::AsyncFuncState::running(
    const Field &base, const std::shared_future<ValAdaptor> &result) {
    return std::make_shared<internal::AsyncFuncState>(base, result);
}
std::shared_ptr<internal::AsyncFuncState>
internal::AsyncFuncState::error(const Field &base,
                                const std::exception_ptr &e) {
    auto state = std::make_shared<internal::AsyncFuncState>(base);
    state->setStarted(true);
    state->setResultException(e);
    return state;
}
std::shared_ptr<internal::AsyncFuncState>
internal::AsyncFuncState::remote(const Field &base, std::size_t caller_id) {
    return std::make_shared<internal::AsyncFuncState>(base, caller_id);
}

void internal::AsyncFuncState::setStarted(bool is_started) {
    started_p.set_value(is_started);
    started_event.operator()(is_started);
    if (!is_started) {
        try {
            throw FuncNotFound(base);
        } catch (...) {
            setResultException(std::current_exception());
        }
    }
}
void internal::AsyncFuncState::setResult(const ValAdaptor &result_val) {
    result_p.set_value(result_val);
    result_event.operator()(result_f);
}
void internal::AsyncFuncState::setResultException(const std::exception_ptr &e) {
    result_p.set_exception(e);
    result_event.operator()(result_f);
}

std::ostream &operator<<(std::ostream &os, const AsyncFuncResult &r) {
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

std::runtime_error &FuncCallHandle::invalidHandle() {
    static std::runtime_error invalid_handle(
        "FuncCallHandle does not have valid "
        "pointer to function call");
    return invalid_handle;
}

template <std::size_t v_index, typename CVal>
std::vector<CVal> &FuncCallHandle::HandleData::initCArgs() {
    if (this->c_args_.index() != v_index) {
        std::vector<CVal> c_args;
        c_args.reserve(this->args_.size());
        for (const auto &a : this->args_) {
            c_args.emplace_back(CVal{
                .as_int = a,
                .as_double = a,
                .as_str = a,
            });
        }
        this->c_args_.emplace<v_index>(std::move(c_args));
    }
    return std::get<v_index>(this->c_args_);
}
const wcfMultiVal *FuncCallHandle::cArgs() const {
    if (handle_data_) {
        auto &c_args = handle_data_->initCArgs<1, wcfMultiVal>();
        return c_args.data();
    } else {
        throw invalidHandle();
    }
}
const wcfMultiValW *FuncCallHandle::cWArgs() const {
    if (handle_data_) {
        auto &c_args = handle_data_->initCArgs<2, wcfMultiValW>();
        return c_args.data();
    } else {
        throw invalidHandle();
    }
}
const std::vector<ValAdaptor> &FuncCallHandle::args() const {
    if (handle_data_) {
        return handle_data_->args_;
    } else {
        throw invalidHandle();
    }
}

void FuncCallHandle::reject(const std::string &message) {
    if (handle_data_) {
        try {
            throw std::runtime_error(message);
        } catch (const std::runtime_error &) {
            handle_data_->result_.set_exception(std::current_exception());
        }
    } else {
        throw invalidHandle();
    }
}
void FuncCallHandle::reject(const std::wstring &message) {
    reject(encoding::toNarrow(message));
}

WEBCFACE_NS_END
