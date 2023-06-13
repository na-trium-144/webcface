#include "client.h"
#include "../message/message.h"

#include <iostream>

namespace WebCFace::Server {
void Client::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
    switch (kind) {
    case MessageKind::name:
        name = std::any_cast<Name>(obj);
        std::cout << "connected from " << name.name << std::endl;
        break;
    }
}
} // namespace WebCFace::Server
