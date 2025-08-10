#include <iostream>
#include <thread>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/view.h>
#include <webcface/func.h>

webcface::Client wcli("tutorial");
// loggerOStream() は std::cout と同様に文字列を出力して使うことができる
std::ostream &logger = wcli.loggerOStream();

int hoge() {
    logger << "Function hoge started" << std::endl;
    return 42;
}
int fuga(int a, std::string_view b) {
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

        webcface::View v = wcli.view("sample");
        v << "Hello, world!" << std::endl; // テキスト表示
        v << "i = " << i << std::endl;
        v << webcface::button("hoge", hoge) << std::endl; // ボタン
        static webcface::InputRef ref_str;
        v << webcface::textInput("str").bind(ref_str); // 文字列入力
        v << webcface::button("print", [] {
            // クリックすると、入力した文字列を表示
            logger << "str = " << ref_str.asString() << std::endl;
        });
        v << std::endl;

        wcli.sync(); // wcli に書き込んだデータを送信する
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}