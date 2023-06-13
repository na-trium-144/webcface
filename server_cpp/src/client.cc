#include "client.h"
#include "message.h"
namespace WebCFace::Server {
void Client::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
    switch (kind) {
    case MessageKind::name:
        name = std::any_cast<std::string>(obj);
        break;
    }
}
} // namespace WebCFace::Server
