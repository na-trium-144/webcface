#include <chrono>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <webcface/webcface.hpp>

void func1()
{
    std::cout << "func1" << std::endl;
}
void func2(int a)
{
    std::cout << "func2 " << a << std::endl;
}

int main()
{
    WebCFace::initStdLogger();
    int t = 0;
    WebCFace::startServer(3001);

    // このように書くと ブラウザのシェル関数のところにshell1が現れる
    WebCFace::addFunctionToRobot("shell1", []() { std::cout << "シェル1" << std::endl; });
    WebCFace::addFunctionToRobot("func1", func1);
    WebCFace::addFunctionToRobot("func2", func2);
    WebCFace::addFunctionToRobot("long_function", [](){
        std::cout << "long_function start" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "end" << std::endl;
    });
    WebCFace::addSharedVarToRobot("t", t);

    // 引数を設定するときは、第３引数に引数の変数名を初期化子リストの形式で書く
    WebCFace::addFunctionToRobot(
        "shell2", [](int a) { std::cout << "シェル2 " << a << std::endl; }, {"intの引数"});
    WebCFace::addFunctionToRobot(
        "shell3", [](bool a) { std::cout << "シェル3 " << a << std::endl; }, {"boolの引数"});
    WebCFace::addFunctionToRobot("shell4",
        [](std::string a) { std::cout << "シェル4 " << a << std::endl; }, {"stringの引数"});
    WebCFace::addFunctionToRobot("shell5",
        [](double value, bool flag, std::string name) {
            std::cout << "シェル5 " << value << "," << flag << "," << name << std::endl;
        },
        {"value(double)", "flag(bool)", "name(string)"},
        {3, false, "あああああ"});


    /* WebCFace::addFunctionFromRobot("fakesensor1", []() { return 1; }); */
    WebCFace::addSharedVarFromRobot("fakesensor2", t);
    /* WebCFace::addFunctionFromRobot("fakesensor3", []() -> std::string { return "Hello,World!"; }); */

    // 以下のように書くと、ブラウザにでーたを送信する頻度を最高10Hzに制限することができる
    // WebCFace::setMaxSendDataFrequency(10);

    {
        using namespace WebCFace::Layout;
        using namespace WebCFace::Literals;
        // clang-format off
        Stack s{{
            {{ Button("func1"_callback), Button("fucn1", "func1"_callback), }},
            Button("shell1"_callback),
            Button("hoge"_callback)
        }};
        WebCFace::addPageLayout("test", {{
            {{ Button("func1"_callback), Button("func1"_callback).MuiColor("secondary"), s }},
            {{ Button("関数直接指定", func1), }},
            {{ "tの値は", "fakesensor2"_value, [&](){return t;}, Button("fakesensor2"_value, "hoge"_callback), }}
        }});
        // clang-format on
    }

    while (true) {
        {
            std::lock_guard l(WebCFace::callback_mutex);
            t++;
            WebCFace::sendData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
