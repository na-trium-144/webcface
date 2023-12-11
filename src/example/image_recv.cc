#include <webcface/webcface.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <thread>

int main() {
    webcface::Client wcli;
    wcli.waitConnection();

    while(true){
        webcface::ImageData img = wcli.member("example_image_send").image("sample").get();
        if(!img.mat().empty()){
            cv::imshow("Display window (image_recv)", img.mat());
            cv::waitKey(0); // Wait for a keystroke in the window
            cv::imwrite("recv_image.png", img.mat());
            return 0;
        }
        std::this_thread::yield();
    }
}