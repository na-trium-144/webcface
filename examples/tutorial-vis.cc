#include <iostream>
#include <thread>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>

webcface::Client wcli("tutorial");
// loggerOStream() は std::cout と同様に文字列を出力して使うことができる
std::ostream &logger = wcli.loggerOStream();

int hoge() {
    logger << "Function hoge started" << std::endl;
    return 42;
}
int fuga(int a, const std::string &b) {
    logger << "Function fuga(" << a << ", " << b << ") started" << std::endl;
    return a;
}

int main() {
    wcli.func("hoge").set(hoge);
    wcli.func("fuga").set(fuga).setArgs({
        webcface::Arg("a").init(100),
        webcface::Arg("b").option({"foo", "bar", "baz"}),
    });

    wcli.waitConnection();

    logger << "Hello, World!" << std::endl;

    int i = 0;

    while (true) {
        i++;
        wcli.value("hoge") = i; // 「hoge」という名前のvalueに値をセット
        if (i % 2 == 0) {
            wcli.text("fuga") = "even";
            // 「fuga」という名前のtextに文字列をセット
        } else {
            wcli.text("fuga") = "odd";
        }

        wcli.sync(); // wcli に書き込んだデータを送信する
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}