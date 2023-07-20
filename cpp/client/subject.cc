#include <webcface/webcface.h>
#include <string>

namespace WebCFace {
SubjectClient::SubjectClient(Client *cli, const std::string &subject)
    : cli(cli), entry_store(cli->entry_store), value_store(cli->value_store),
      text_store(cli->text_store), func_store(cli->func_store),
      func_impl_store(cli->func_impl_store), subject(subject) {}
} // namespace WebCFace