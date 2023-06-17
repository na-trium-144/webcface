#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>

#include <string>
#include <webcface/webcface.hpp>
#include <thread>
#include <chrono>
#include <drogon/WebSocketClient.h>
#include <drogon/drogon.h>
#include <future>
#include <iostream>
#include <array>

int var1 = 333;
double var2 = 66.66;
bool var3 = true;
std::string var4 = "hello";
int var5 = 0;
std::array<int, 3> var_array = {1, 2, 3};
const std::string hello_world_log = "Hello, World! aaaaaaa";
const std::string hello_world_log2 = "hello, world! bbbbbbb";

constexpr int port = 60000;

DROGON_TEST(websocket)
{
    using namespace WebCFace;
    auto cli = drogon::WebSocketClient::newWebSocketClient("127.0.0.1", port);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/");
    std::promise<bool> p_connection_ok;
    std::array<bool, 10> status_received = {};
    bool hello_world_received1 = false;
    bool hello_world_received2 = false;
    std::cout << hello_world_log << std::endl;   // client接続前のcout
    std::cerr << hello_world_log2 << std::endl;  // client接続前のcerr

    cli->setMessageHandler([&status_received, &hello_world_received1, &hello_world_received2,
                               TEST_CTX](const std::string& msg, const drogon::WebSocketClientPtr&,
                               const drogon::WebSocketMessageType&) {
        Json::Value parsed(Json::objectValue);
        Json::Reader reader;
        reader.parse(msg, parsed);
        printf("msg: %s\n", msg.c_str());
        if (parsed["msgname"] == "setting") {
            // 判定するのめんどい
        }
        if (parsed["msgname"] == "from_robot") {
            auto msg = parsed["msg"];
// 値があっているか確認
#define varCheck(i, name, var)                               \
    if (msg.isMember(name)) {                                \
        CHECK(deserialize<decltype(var)>(msg[name]) == var); \
        status_received[i] = true;                           \
    }
            varCheck(0, "status1", var1);
            varCheck(1, "status2.unknown0", var1);
            varCheck(2, "status2.unknown1", var2);
            varCheck(3, "status2.unknown2", var3);
            varCheck(4, "status2.unknown3", var4);
            varCheck(5, "status3", var1);
            varCheck(6, "status4.a", var5);
            varCheck(7, "status4.b", var5);
            varCheck(8, "status4.c", var5);
            varCheck(9, "status5", var_array);
        }
        if (parsed["msgname"] == "log") {
            auto msg = parsed["msg"];
            for (int i = 0; i < msg.size(); i++) {
                if (deserialize<std::string>(msg[i]["text"]) == hello_world_log) {
                    hello_world_received1 = true;
                }
                if (deserialize<std::string>(msg[i]["text"]) == hello_world_log2) {
                    hello_world_received2 = true;
                }
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    cli->connectToServer(
        req, [&p_connection_ok, TEST_CTX](const drogon::ReqResult& reqresult,
                 const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& cli) {
            CHECK(reqresult == drogon::ReqResult::Ok);
            p_connection_ok.set_value(reqresult == drogon::ReqResult::Ok);
        });
    if (p_connection_ok.get_future().get()) {

        WebCFace::addValueFromRobot("status4", {"a", "b", "c"}, var5, var5, var5);
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        CHECK(
            status_received
            == (std::array<bool, 10>{true, true, true, true, true, true, true, true, true, true}));
        CHECK(hello_world_received1);
        CHECK(hello_world_received2);

        status_received.fill(false);
        hello_world_received1 = false;
        hello_world_received2 = false;
        var1++;
        var2++;
        var3 = !var3;
        var4 = "hogehoge";
        var5++;
        var_array = {4, 5, 6};
        std::cout << hello_world_log << std::endl;   // 接続後のcout
        std::cerr << hello_world_log2 << std::endl;  // 接続後のcerr

        WebCFace::addValueFromRobot("status4", {"a", "b", "c"}, var5, var5, var5);
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        WebCFace::sendData();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        CHECK(
            status_received
            == (std::array<bool, 10>{true, true, true, true, true, true, true, true, true, true}));
        CHECK(hello_world_received1);
        CHECK(hello_world_received2);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    cli->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main(int argc, char** argv)
{
    WebCFace::initStdLogger();
    WebCFace::startServer(port);

    WebCFace::addSharedVarFromRobot("status1", var1);
    WebCFace::addSharedVarFromRobot("status2", {}, var1, var2, var3, var4);
    WebCFace::addFunctionFromRobot("status3", []() { return var1; });
    WebCFace::addSharedVarFromRobot("status5", var_array);

    int status = drogon::test::run(argc, argv);
    // WebCFace::quitServer();
    return status;
}
