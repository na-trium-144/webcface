#include <webcface/webcface.h>
#include <string>

namespace WebCFace {
Member::Member(Client *cli, const std::string &name) : cli(cli), name_(name) {}

std::vector<Value> Member::values() const {
    auto keys = cli->value_store.getEntry(this->name());
    std::vector<Value> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = value(keys[i]);
    }
    return ret;
}
std::vector<Text> Member::texts() const {
    auto keys = cli->text_store.getEntry(this->name());
    std::vector<Text> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = text(keys[i]);
    }
    return ret;
}
std::vector<Func> Member::funcs() const {
    auto keys = cli->func_store.getEntry(this->name());
    std::vector<Func> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = func(keys[i]);
    }
    return ret;
}

MemberEvent<Value> Member::valuesChange() {
    return MemberEvent<Value>{&cli->value_entry_event};
}
MemberEvent<Text> Member::textsChange() {
    return MemberEvent<Text>{&cli->text_entry_event};
}
MemberEvent<Func> Member::funcsChange() {
    return MemberEvent<Func>{&cli->func_entry_event};
}

} // namespace WebCFace