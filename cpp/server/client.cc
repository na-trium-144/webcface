#include "client.h"
#include "../message/message.h"

#include <iostream>

namespace WebCFace::Server {
void Client::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
    switch (kind) {
    case MessageKind::name:
        name = std::any_cast<Name>(obj).name;
        std::cout << "connected from " << name << std::endl;
        break;
    case MessageKind::value:
        std::cout << "value " << std::any_cast<Value>(obj).data << std::endl;
        break;
    }
}
} // namespace WebCFace::Server
