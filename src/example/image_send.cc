#include <webcface/client.h>
#include <webcface/image.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <iostream>

int main(int argc, const char **argv) {
    if (argc <= 1) {
        std::cerr << "no image file specified" << std::endl;
        return 1;
    }
    webcface::Client wcli("example_image_send");

    cv::Mat img = cv::imread(argv[1], cv::IMREAD_COLOR);
    cv::resize(img, img, cv::Size(600, 600));
    if (img.empty()) {
        std::cout << "Could not read the image: " << argv[1] << std::endl;
        return 1;
    }
    wcli.image("sample").set(img);
    wcli.sync();
    cv::imshow("Display window (image_send)", img);
    cv::waitKey(0); // Wait for a keystroke in the window
    cv::imwrite("send_image.png", img);
}
