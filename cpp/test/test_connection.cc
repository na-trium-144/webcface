#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include "utils.h"

DROGON_TEST(ConnectionTest) {
    WebCFace::Client cli1("test1");
    wait();
    CHECK(cli1.connected());

    // cli1.close();
    // wait();
    // CHECK(!cli1.connected());
}