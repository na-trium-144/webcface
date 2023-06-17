#ifndef WEBCFACE_OPENCV_FOUND
#include <iostream>
int main()
{
    std::cerr << "OpenCV is disabled in cmake option" << std::endl;
    return 1;
}
#else

#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <webcface/webcface.hpp>
#include <webcface/external/opencv.hpp>

constexpr float width = 640.0;

int main()
{
    cv::Mat img;
    cv::VideoCapture cap(0);
    WebCFace::initStdLogger();
    WebCFace::startServer(3001);
    WebCFace::addFunctionToRobot("shell1", []() { std::cout << "シェル1" << std::endl; }, {});
    while (true) {
        cap >> img;
        const float height = width * img.rows / img.cols;
        cv::resize(img, img, cv::Size(), width / img.cols, height / img.cols);
        cv::imshow("image", img);
        WebCFace::addImage("image1", img);
        WebCFace::sendData();
        const auto key = cv::waitKey(100);
        if (key == 27) {
            WebCFace::quitServer();
            return 0;
        }
    }
}
#endif