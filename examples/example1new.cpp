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

    using namespace WebCFace::Literals;

    // このように書くと ブラウザのシェル関数のところにshell1が現れる
    "shell1"_callback = []() { std::cout << "シェル1" << std::endl; };
    "func1"_callback = func1;
    "func2"_callback = func2;
    // "t"_callback = t; todo: sharedvartorobot

    // 引数を設定するときは、第３引数に引数の変数名を初期化子リストの形式で書く
    "shell2"_callback.arg("intの引数") = [](int a) { std::cout << "シェル2 " << a << std::endl; };
    "shell3"_callback.arg("boolの引数") = [](bool a) { std::cout << "シェル3 " << a << std::endl; };
    "shell4"_callback.arg("stringの引数")
        = [](std::string a) { std::cout << "シェル4 " << a << std::endl; };
    "shell5"_callback.arg("value(double)", "flag(bool)", "name(string)")
        .default_value(4, true, "いいいいい")
        = [](int value, bool flag, std::string name) {
              std::cout << "シェル5 " << value << "," << flag << "," << name << std::endl;
          };
    "dialog"_callback = [] { WebCFace::dialog("test"); };


    "fakesensor1"_value = 1;
    "fakesensor2"_value = &t;
    "fakesensor3"_value = []() -> std::string { return "Hello,World!"; };

    // 以下のように書くと、ブラウザにでーたを送信する頻度を最高10Hzに制限することができる
    // WebCFace::setMaxSendDataFrequency(10);

    {
        using namespace WebCFace::Layout;
        // clang-format off
            Stack s{{
                {{ Button("func1"_callback), Button("fucn1", "func1"_callback), }},
                Button("shell1"_callback),
                Button("hoge"_callback),
            }};
            WebCFace::addPageLayout("test", {{
                {{ Button("func1"_callback), Button("func1"_callback).MuiColor("secondary"), s }},
                {{ Button("関数直接指定", func1), }},
                {{ Button("関数直接指定", "func1direct"_callback = func1), }},
                {{ 
                    "tの値は", "fakesensor2"_value, [&](){return t;}, &t,
                     "t_direct"_value = [&](){return t;}, Button("fakesensor2"_value,"hoge"_callback), 
                }},
                {{
                    Button("shell5"_callback.arg(1, false, "button1")),
                    Button("shell5"_callback.arg(2, false, "button2")),
                    Button("shell5"_callback << 3 << false << "button3"),
                }},
            }});
        // clang-format on
    }

    while (true) {
        {
            std::lock_guard l(WebCFace::callback_mutex);
            t++;
            "fakesensor2v"_value = t;
            using namespace WebCFace::Layout;
            PageLayout p("test2");
            p.clear();
            p << Button("func1"_callback) << Button("func1"_callback).MuiColor("secondary")
              << p.endl;
            p << "tの値は"
              << "fakesensor2"_value << t << Button(t, "hoge"_callback) << p.endl;
            p << Alert(t) << Alert(t, "info") << p.endl;
            for (int i = 0; i < 5; i++) {
                p << Button("func2 " + std::to_string(i), "func2"_callback << i);
            }
            p << p.endl;
            Drawing aaa(100, 100);
            auto layer = aaa.createLayer("aaa");
            layer.drawRect(10, 10, 90, 90, "red");
            for (int x = 20; x < 100; x += 20) {
                layer.drawCircle(x, 50, 5, "blue");
            }
            p << aaa << p.endl;  // 全部書き終わってからページに入れること
            WebCFace::sendData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
