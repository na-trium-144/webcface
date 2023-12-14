#include <webcface/webcface.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <thread>
#include <iostream>

int main() {
    webcface::Client wcli;
    wcli.waitConnection();

    // image_sendを5回起動して終了すると5通りのリクエストをして表示するサンプル

    webcface::Image img = wcli.member("example_image_send").image("sample");
    img.request();
    img.appendListener([](auto) { std::cout << "image updated" << std::endl; });
    while (true) {
        // auto mat = wcli.member("example_image_send").image("sample").mat();
        // NG
        auto mat2 = img.mat(); // OK
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            cv::imwrite("recv_image.png", img.mat());
            break;
        }
        std::this_thread::yield();
    }
    img.request(300, 300);
    while (true) {
        auto mat2 = img.mat(); // OK
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, 300x300)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
    img.request(std::nullopt, std::nullopt, 1);
    while (true) {
        auto mat2 = img.mat(); // OK
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, gray)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
    img.request(std::nullopt, std::nullopt, 3,
                webcface::ImageCompressMode::jpeg, 20);
    while (true) {
        auto mat2 = img.mat(); // OK
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, jpeg)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
    img.request(std::nullopt, std::nullopt, 3, webcface::ImageCompressMode::png,
                1);
    while (true) {
        auto mat2 = img.mat(); // OK
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, png)", img.mat());
            cv::waitKey(0); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
}