#include <webcface/webcface.h>
#include <string>

namespace WebCFace {
Member::Member(Client *cli, const std::string &name) : cli(cli), name_(name) {}

EventTarget<Value> Member::valuesChange() const {
    return EventTarget<Value>{EventType::value_entry, cli, &cli->event_queue,
                              name_};
}
EventTarget<Text> Member::textsChange() const {
    return EventTarget<Text>{EventType::text_entry, cli, &cli->event_queue,
                             name_};
}
EventTarget<Func> Member::funcsChange() const {
    return EventTarget<Func>{EventType::func_entry, cli, &cli->event_queue,
                             name_};
}

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

} // namespace WebCFace