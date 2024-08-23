#include <cassert>
#include <webcface/client.h>
#include <webcface/image.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <thread>
#include <iostream>

webcface::Client wcli;

void img_update1(const webcface::Image &img);
void img_update2(const webcface::Image &img);
void img_update3(const webcface::Image &img);
void img_update4(const webcface::Image &img);
void img_update5(const webcface::Image &img);

int main() {
    wcli.waitConnection();

    // image_sendを5回起動して終了すると5通りのリクエストをして表示するサンプル

    const webcface::Image &img = wcli.member("example_image_send").image("sample");
    img.request();
    img.onChange(img_update1);
    wcli.loopSync();
}

void img_update1(const webcface::Image &img) {
    std::cout << "image updated" << std::endl;
    auto img_frame = img.get();
    if (!img_frame.empty()) {
        assert(img_frame.channels() == 3);
        assert(img_frame.color_mode() == webcface::ImageColorMode::bgr);
        cv::Mat mat2(img_frame.rows(), img_frame.cols(), CV_8UC3,
                     img_frame.data().data());
        cv::imshow("Display window (image_recv)", mat2);
        cv::waitKey(1); // Wait for a keystroke in the window
        cv::imwrite("recv_image.png", mat2);

        img.request(webcface::sizeWH(300, 300));
        img.onChange(img_update2);
    }
}
void img_update2(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty()) {
        cv::Mat mat2(img_frame.rows(), img_frame.cols(), CV_8UC3,
                     img_frame.data().data());
        assert(img_frame.channels() == 3);
        assert(img_frame.color_mode() == webcface::ImageColorMode::bgr);
        assert(img_frame.rows() == 300);
        assert(img_frame.cols() == 300);
        cv::imshow("Display window (image_recv, 300x300)", mat2);
        cv::waitKey(1); // Wait for a keystroke in the window

        img.request(std::nullopt, webcface::ImageColorMode::gray);
        img.onChange(img_update3);
    }
}
void img_update3(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty()) {
        assert(img_frame.channels() == 1);
        assert(img_frame.color_mode() == webcface::ImageColorMode::gray);
        cv::Mat mat2(img_frame.rows(), img_frame.cols(), CV_8UC1,
                     img_frame.data().data());
        cv::imshow("Display window (image_recv, gray)", mat2);
        cv::waitKey(1); // Wait for a keystroke in the window

        img.request(std::nullopt, webcface::ImageCompressMode::jpeg, 20);
        img.onChange(img_update4);
    }
}
void img_update4(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty()) {
        cv::Mat mat2 = cv::imdecode(img_frame.data(), cv::IMREAD_COLOR);
        cv::imshow("Display window (image_recv, jpeg)", mat2);
        cv::waitKey(1); // Wait for a keystroke in the window

        img.request(std::nullopt, webcface::ImageCompressMode::png, 1);
        img.onChange(img_update5);
    }
}
void img_update5(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty()) {
        cv::Mat mat2 = cv::imdecode(img_frame.data(), cv::IMREAD_COLOR);
        cv::imshow("Display window (image_recv, png)", mat2);
        cv::waitKey(0); // Wait for a keystroke in the window

        wcli.close();
    }
}
