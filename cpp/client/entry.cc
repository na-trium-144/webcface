#include "entry.h"
#include <webcface/webcface.h>

namespace WebCFace {
Value Client::value(const std::string &name) { return value("", name); }
Value Client::value(const std::string &from, const std::string &name) {
    return Value{this, from, name};
}

Value &Value::set(double data) {
    cli->value_send[name] = data;
    return *this;
}

std::optional<double> Value::try_get() const {
    if (from == "") {
        auto it = cli->value_send.find(name);
        if (it != cli->value_send.end()) {
            return it->second;
        }
    }else{
        // cli->subscribe(from, name);
        auto s_it = cli->value_recv.find(from);
        if(s_it != cli->value_recv.end()){
            auto it = s_it->second.find(name);
            if(it != s_it->second.end()){
                return it->second;
            }
        }
    }
    return std::nullopt;
}
double Value::get() const {
    auto v = try_get();
    if(v){
        return *v;
    }else{
        return 0;
    }
}
} // namespace WebCFace