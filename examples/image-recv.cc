#include <webcface/client.h>
#include <webcface/image.h>
#include <thread>
#include <iostream>
#include <Magick++.h>
#include <fstream>
#include <cassert>

int main() {
    Magick::InitializeMagick(nullptr);
    webcface::Client wcli;
    wcli.waitConnection();

    webcface::Image img = wcli.member("example_image_send").image("sample");
    img.request();
    img.onChange([](auto) { std::cout << "image updated" << std::endl; });
    while (true) {
        wcli.waitRecv();
        auto img_frame = img.get();
        if (!img_frame.empty()) {
            std::cout << "1. Normal" << std::endl;
            assert(img_frame.channels() == 3);
            assert(img_frame.color_mode() == webcface::ImageColorMode::rgb);
            Magick::Image m(img_frame.rows(), img_frame.cols(), "RGB",
                            Magick::CharPixel, img_frame.data().data());
            // m.display();
            m.write("webcface-example-image-recv-1.png");
            break;
        }
    }
    img.request(webcface::sizeWH(300, 300));
    while (true) {
        wcli.waitRecv();
        // auto mat2 = img.mat(); // OK
        auto img_frame = img.get();
        if (!img_frame.empty()) {
            std::cout << "2. Resized to 300x300" << std::endl;
            assert(img_frame.channels() == 3);
            assert(img_frame.color_mode() == webcface::ImageColorMode::rgb);
            assert(img_frame.rows() == 300);
            assert(img_frame.cols() == 300);
            Magick::Image m(img_frame.rows(), img_frame.cols(), "RGB",
                            Magick::CharPixel, img_frame.data().data());
            // m.display();
            m.write("webcface-example-image-recv-2.png");
            break;
        }
    }
    img.request(std::nullopt, webcface::ImageColorMode::gray);
    while (true) {
        wcli.waitRecv();
        // auto mat2 = img.mat(); // OK
        auto img_frame = img.get();
        if (!img_frame.empty()) {
            std::cout << "3. Grayscale" << std::endl;
            assert(img_frame.channels() == 1);
            assert(img_frame.color_mode() == webcface::ImageColorMode::gray);
            Magick::Image m(img_frame.rows(), img_frame.cols(), "K",
                            Magick::CharPixel, img_frame.data().data());
            m.type(Magick::TrueColorType);
            m.negate(true);
            // m.display();
            m.write("webcface-example-image-recv-3.jpg");
            break;
        }
        wcli.waitRecv();
    }
    img.request(std::nullopt, webcface::ImageCompressMode::jpeg, 20);
    while (true) {
        wcli.waitRecv();
        // auto mat2 = img.mat(); // OK
        auto img_frame = img.get();
        if (!img_frame.empty()) {
            std::cout << "4. JPEG" << std::endl;
            // Magick::Blob blob(img_frame.data().data(),
            // img_frame.data().size()); Magick::Image img(blob,
            // {img_frame.cols(), img_frame.rows()}, "JPEG"); img.display();
            std::ofstream ofs("webcface-example-image-recv-4.jpg",
                              std::ios_base::out | std::ios_base::binary);
            ofs.write(reinterpret_cast<const char *>(img_frame.data().data()),
                      static_cast<int>(img_frame.data().size()));
            break;
        }
    }
    img.request(std::nullopt, webcface::ImageCompressMode::png, 1);
    while (true) {
        wcli.waitRecv();
        // auto mat2 = img.mat(); // OK
        auto img_frame = img.get();
        if (!img_frame.empty()) {
            std::cout << "5. PNG" << std::endl;
            // Magick::Blob blob(img_frame.data().data(),
            // img_frame.data().size()); Magick::Image img(blob,
            // {img_frame.cols(), img_frame.rows()}, "PNG"); img.display();
            std::ofstream ofs("webcface-example-image-recv-5.png",
                              std::ios_base::out | std::ios_base::binary);
            ofs.write(reinterpret_cast<const char *>(img_frame.data().data()),
                      static_cast<int>(img_frame.data().size()));
            break;
        }
    }
    std::cout << "Images are in current directory." << std::endl;
}
