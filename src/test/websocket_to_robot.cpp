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

bool func1_received = false, func3_received = false;
void func1()
{
    func1_received = true;
}
int func2_received = 0, func4_received = 0;
void func2(int a)
{
    func2_received = a;
}

constexpr int func_testvalue = 777;
constexpr double var2_testvalue = 777.777;
constexpr bool var3_testvalue = false;
const std::string var4_testvalue = "hello";
int var1 = 333;
double var2 = 0;
bool var3 = !var3_testvalue;
std::string var4 = "";
std::array<int, 3> var_array_testvalue = {1, 2, 3}, var_array = {};

constexpr int port = 60000;

DROGON_TEST(websocket)
{
    using namespace WebCFace;
    auto cli = drogon::WebSocketClient::newWebSocketClient("127.0.0.1", port);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/");
    std::promise<bool> p_connection_ok;
    bool hello_world_received = false;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    cli->connectToServer(
        req, [&p_connection_ok, TEST_CTX](const drogon::ReqResult& reqresult,
                 const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& cli) {
            CHECK(reqresult == drogon::ReqResult::Ok);
            p_connection_ok.set_value(reqresult == drogon::ReqResult::Ok);
        });
    if (p_connection_ok.get_future().get()) {

        cli->getConnection()->send(R"({
            "msgname":"function",
            "msg":{"name":"func1","args":{}}
        })");
        cli->getConnection()->send(R"({
            "msgname":"function",
            "msg":{
                "name":"func2","args":{
                    "unknown0": )" + std::to_string(func_testvalue)
                                   + R"(
                }
            }
        })");
        cli->getConnection()->send(R"({
            "msgname":"function",
            "msg":{"name":"func3","args":{}}
        })");
        cli->getConnection()->send(R"({
            "msgname":"function",
            "msg":{
                "name":"func4",
                "args":{
                    "a": )" + std::to_string(func_testvalue)
                                   + R"(
                }
            }
        })");
        cli->getConnection()->send(R"({
            "msgname":"to_robot",
            "msg":{
                "name":"var1",
                "value": {
                    "var1": )" + std::to_string(func_testvalue)
                                   + R"(
                }
            }
        })");
        cli->getConnection()->send(R"({
            "msgname":"to_robot",
            "msg":{
                "name":"var_multi",
                "value":{
                    "unknown0": )" + std::to_string(var2_testvalue)
                                   + R"(,
                    "unknown1": )" + std::to_string(var3_testvalue)
                                   + R"(,
                    "unknown2": ")" + var4_testvalue
                                   + R"("
                }
            }
        })");
        cli->getConnection()->send(R"({
            "msgname":"to_robot",
            "msg":{
                "name":"var_array",
                "value":{
                    "var_array": [
                        )" + std::to_string(var_array_testvalue[0])
                                   + R"(,
                        )" + std::to_string(var_array_testvalue[1])
                                   + R"(,
                        )" + std::to_string(var_array_testvalue[2])
                                   + R"(
                    ]
                }
            }
        })");

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        CHECK(func1_received);
        CHECK(func2_received == func_testvalue);
        CHECK(func3_received);
        CHECK(func4_received == func_testvalue);
        CHECK(var1 == func_testvalue);
        CHECK(var2 == var2_testvalue);
        CHECK(var3 == var3_testvalue);
        CHECK(var4 == var4_testvalue);
        CHECK(var_array == var_array_testvalue);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    cli->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main(int argc, char** argv)
{
    WebCFace::initStdLogger();
    WebCFace::startServer(port);

    WebCFace::addFunctionToRobot("func1", func1);
    WebCFace::addFunctionToRobot("func2", func2);
    WebCFace::addFunctionToRobot("func3", []() { func3_received = true; });
    WebCFace::addFunctionToRobot("func4", [](int a) { func4_received = a; }, {"a"});
    WebCFace::addSharedVarToRobot("var1", var1);
    WebCFace::addSharedVarToRobot("var_multi", {}, var2, var3, var4);
    WebCFace::addSharedVarToRobot("var_array", var_array);

    int status = drogon::test::run(argc, argv);
    // WebCFace::quitServer();
    return status;
}
