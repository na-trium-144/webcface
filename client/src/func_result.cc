#include <webcface/func_result.h>

WEBCFACE_NS_BEGIN

FuncNotFound::FuncNotFound(const FieldBase &base)
    : std::runtime_error("member(\"" + base.member_.decode() + "\")" +
                         ".func(\"" + base.field_.decode() + "\") is not set") {
}

eventpp::CallbackList<void(bool)> &AsyncFuncResult::onStarted() const {
    if (!started_event) {
        throw std::runtime_error("started event is null");
    }
    return *started_event;
}
eventpp::CallbackList<void(std::shared_future<ValAdaptor>)> &
AsyncFuncResult::onResult() const {
    if (!result_event) {
        throw std::runtime_error("result event is null");
    }
    return *result_event;
}

AsyncFuncResultSetter::AsyncFuncResultSetter(const Field &base)
    : Field(base), started(), result(), started_f(started.get_future().share()),
      result_f(result.get_future().share()),
      started_event(std::make_shared<decltype(started_event)::element_type>()),
      result_event(std::make_shared<decltype(result_event)::element_type>()) {}
void AsyncFuncResultSetter::setStarted(bool is_started) {
    started.set_value(is_started);
    started_event->operator()(is_started);
    if (!is_started) {
        try {
            throw FuncNotFound(*this);
        } catch (...) {
            setResultException(std::current_exception());
        }
    }
}
void AsyncFuncResultSetter::setResult(const ValAdaptor &result_val) {
    result.set_value(result_val);
    result_event->operator()(result_f);
}
void AsyncFuncResultSetter::setResultException(const std::exception_ptr &e) {
    result.set_exception(e);
    result_event->operator()(result_f);
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
