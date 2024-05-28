#include <webcface/client.h>
#include <webcface/image.h>
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
        // auto mat2 = img.mat(); // OK
        auto img_frame = img.get();
        assert(img_frame.channels() == 3);
        assert(img_frame.color_mode() == webcface::ImageColorMode::bgr);
        cv::Mat mat2(img_frame.rows(), img_frame.cols(), CV_8UC3, img_frame.data().data());
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
        // auto mat2 = img.mat(); // OK
        auto img_frame = img.get();
        assert(img_frame.channels() == 3);
        assert(img_frame.color_mode() == webcface::ImageColorMode::bgr);
        assert(img_frame.rows() == 300);
        assert(img_frame.cols() == 300);
        cv::Mat mat2(img_frame.rows(), img_frame.cols(), CV_8UC3, img_frame.data().data());
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, 300x300)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
    img.request(std::nullopt, std::nullopt, webcface::ImageColorMode::gray);
    while(true){
        // auto mat2 = img.mat(); // OK
        assert(img_frame.channels() == 1);
        assert(img_frame.color_mode() == webcface::ImageColorMode::gray);
        cv::Mat mat2(img_frame.rows(), img_frame.cols(), CV_8UC1, img_frame.data().data());
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, gray)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
    img.request(std::nullopt, std::nullopt, webcface::ImageCompressMode::jpeg, 20);
    while(true){
        // auto mat2 = img.mat(); // OK
        cv::Mat mat2 = cv::imdecode(img.data().data(), cv::IMREAD_COLOR);
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, jpeg)", img.mat());
            cv::waitKey(1); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
    img.request(std::nullopt, std::nullopt, webcface::ImageCompressMode::png, 1);
    while(true){
        // auto mat2 = img.mat(); // OK
        cv::Mat mat2 = cv::imdecode(img.data().data(), cv::IMREAD_COLOR);
        if (!mat2.empty()) {
            cv::imshow("Display window (image_recv, png)", img.mat());
            cv::waitKey(0); // Wait for a keystroke in the window
            break;
        }
        std::this_thread::yield();
    }
}
