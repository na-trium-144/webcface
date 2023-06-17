#pragma once
#include <opencv2/opencv.hpp>
#include <drogon/utils/Utilities.h>
#include "../registration.hpp"

namespace WebCFace
{

inline void addImage(const std::string& name, const cv::Mat& img)
{
    std::string encoded = "";
    if (img.empty()) {
        std::cerr << "image is empty!!!" << std::endl;
    } else {
        std::vector<uchar> buf;
        cv::imencode(".jpg", img, buf);
        encoded = "data:image/jpeg;base64," + drogon::utils::base64Encode(buf.data(), buf.size());
    }
    addImage(name, encoded);
}

}  // namespace WebCFace
