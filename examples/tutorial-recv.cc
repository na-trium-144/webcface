#include <iostream>
#include <thread>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>

webcface::Client wcli("tutorial-recv");

int main() {
    wcli.waitConnection();

    std::cout << "Hello, World! (receiver)" << std::endl;

    webcface::Member sender = wcli.member("tutorial-send");
    sender.value("data").onChange([](const webcface::Value &v) {
        std::cout << "data changed: " << v.get() << std::endl;
    });
    sender.text("message").onChange([](const webcface::Text &t) {
        std::cout << "message changed: " << t.get() << std::endl;
    });

    while (true) {
        webcface::Promise fuga_p = sender.func("fuga").runAsync(123, "abc");
        fuga_p.onFinish([fuga_p](){
            // fuga_pが完了したとき、結果を表示します
            if(fuga_p.isError()){
                std::cout << "Error in fuga(123, abc): " << fuga_p.rejection() << std::endl;
            }else{
                std::cout << "fuga(123, abc) = " << fuga_p.response().asInt() << std::endl;
            }
        });

        wcli.sync();

        std::optional<double> data =
            wcli.member("tutorial-send").value("data").tryGet();
        if (data.has_value()) {
            std::cout << "data = " << *data << std::endl;
        } else {
            std::cout << "data is null" << std::endl;
        }

        std::optional<std::string> message =
            wcli.member("tutorial-send").text("message").tryGet();
        if (message.has_value()) {
            std::cout << "message = " << *message << std::endl;
        } else {
            std::cout << "message is null" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
