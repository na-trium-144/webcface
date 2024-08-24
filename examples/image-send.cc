#include <webcface/client.h>
#include <webcface/image.h>
#include <string>
#include <iostream>
#include <Magick++.h>

int main() {
    Magick::InitializeMagick(nullptr);
    webcface::Client wcli("example_image_send");

    Magick::Image image("100x100", {255, 255, 255});
    for (int x = -10; x < 10; x++) {
        for (int y = -10; y < 10; y++) {
            if (x * x + y * y < 100) {
                image.pixelColor(50 + x, 50 + y, {255, 0, 0});
            }
        }
    }

    webcface::ImageFrame img_frame(webcface::sizeWH(100, 100),
                                   webcface::ImageColorMode::rgb);
    image.write(0, 0, 100, 100, "RGB", Magick::CharPixel,
                img_frame.data().data());
    wcli.image("sample").set(img_frame);
    wcli.sync();
    std::cout << "Press Enter to Exit" << std::endl;
    std::string wait;
    std::getline(std::cin, wait);
}
