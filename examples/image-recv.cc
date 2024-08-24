#include <webcface/client.h>
#include <webcface/image.h>
#include <thread>
#include <iostream>
#include <Magick++.h>
#include <fstream>
#include <cassert>

webcface::Client wcli;

void img_update1(const webcface::Image &img);
void img_update2(const webcface::Image &img);
void img_update3(const webcface::Image &img);
void img_update4(const webcface::Image &img);
void img_update5(const webcface::Image &img);

int main() {
    Magick::InitializeMagick(nullptr);
    wcli.waitConnection();

    // image_sendを5回起動して終了すると5通りのリクエストをして表示するサンプル

    webcface::Image img = wcli.member("example_image_send").image("sample");
    img.request();
    img.onChange(img_update1);
    wcli.loopSync();
}

void img_update1(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty()) {
        std::cout << "1. Normal" << std::endl;
        assert(img_frame.channels() == 3);
        assert(img_frame.color_mode() == webcface::ImageColorMode::rgb);
        Magick::Image m(img_frame.rows(), img_frame.cols(), "RGB",
                        Magick::CharPixel, img_frame.data().data());
        // m.display();
        m.write("webcface-example-image-recv-1.png");

        img.request(webcface::sizeWH(300, 300));
        img.onChange(img_update2);
    }
}
void img_update2(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty() && img_frame.rows() == 300 &&
        img_frame.cols() == 300) {
        std::cout << "2. Resized to 300x300" << std::endl;
        assert(img_frame.channels() == 3);
        assert(img_frame.color_mode() == webcface::ImageColorMode::rgb);
        Magick::Image m(img_frame.rows(), img_frame.cols(), "RGB",
                        Magick::CharPixel, img_frame.data().data());
        // m.display();
        m.write("webcface-example-image-recv-2.png");

        img.request(std::nullopt, webcface::ImageColorMode::gray);
        img.onChange(img_update3);
    }
}

void img_update3(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty() && img_frame.channels() == 1) {
        std::cout << "3. Grayscale" << std::endl;
        assert(img_frame.channels() == 1);
        assert(img_frame.color_mode() == webcface::ImageColorMode::gray);
        Magick::Image m(img_frame.rows(), img_frame.cols(), "K",
                        Magick::CharPixel, img_frame.data().data());
        m.type(Magick::TrueColorType);
        m.negate(true);
        // m.display();
        m.write("webcface-example-image-recv-3.jpg");

        img.request(std::nullopt, webcface::ImageCompressMode::jpeg, 20);
        img.onChange(img_update4);
    }
}
void img_update4(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty() &&
        img_frame.compress_mode() == webcface::ImageCompressMode::jpeg) {
        std::cout << "4. JPEG" << std::endl;
        // Magick::Blob blob(img_frame.data().data(),
        // img_frame.data().size()); Magick::Image img(blob,
        // {img_frame.cols(), img_frame.rows()}, "JPEG"); img.display();
        std::ofstream ofs("webcface-example-image-recv-4.jpg",
                          std::ios_base::out | std::ios_base::binary);
        ofs.write(reinterpret_cast<const char *>(img_frame.data().data()),
                  static_cast<int>(img_frame.data().size()));

        img.request(std::nullopt, webcface::ImageCompressMode::png, 1);
        img.onChange(img_update5);
    }
}
void img_update5(const webcface::Image &img) {
    auto img_frame = img.get();
    if (!img_frame.empty() &&
        img_frame.compress_mode() == webcface::ImageCompressMode::png) {
        std::cout << "5. PNG" << std::endl;
        // Magick::Blob blob(img_frame.data().data(),
        // img_frame.data().size()); Magick::Image img(blob,
        // {img_frame.cols(), img_frame.rows()}, "PNG"); img.display();
        std::ofstream ofs("webcface-example-image-recv-5.png",
                          std::ios_base::out | std::ios_base::binary);
        ofs.write(reinterpret_cast<const char *>(img_frame.data().data()),
                  static_cast<int>(img_frame.data().size()));

        std::cout << "Images are in current directory." << std::endl;
        wcli.close();
    }
}
