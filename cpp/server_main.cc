#include "../server/websock.h"

int main() {
    // Set HTTP listener address and port
    // todo: 引数で変えられるようにする
    WebCFace::Server::serverRun(7530);
    return 0;
}
