#include <drogon/drogon.h>
#include "./server/store.h"

int main() {
    WebCFace::Server::controllerKeeper();

    // Set HTTP listener address and port
    // todo: 引数で変えられるようにする
    drogon::app().addListener("0.0.0.0", 7530);
    // Load config file
    // drogon::app().loadConfigFile("../config.json");
    // Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
