#include <iostream>
#include <thread>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>

int main() {
    webcface::Client wcli("tutorial");
    wcli.waitConnection();
    std::ostream &logger = wcli.loggerOStream();
    // loggerOStream() は std::cout と同様に文字列を出力して使うことができる

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