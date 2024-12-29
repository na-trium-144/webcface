#include <webcface/client.h>
#include <webcface/image.h>
#include <string>
#include <iostream>
#include <vips/vips8>

int main() {
    VIPS_INIT("");
    webcface::Client wcli("example_image_send");

    vips::VImage image =
        vips::VImage::new_matrix(100, 100).scale().new_from_image(
            {255, 255, 255});
    image.draw_circle({255, 0, 0}, 50, 50, 10);

    std::size_t data_size;
    void *data = image.write_to_memory(&data_size);
    std::cout << data_size << std::endl;
    webcface::ImageFrame img_frame(webcface::sizeWH(100, 100), data,
                                   webcface::ImageColorMode::rgb);
    g_free(data);

    wcli.image("sample").set(img_frame);
    wcli.sync();
    std::cout << "Press Enter to Exit" << std::endl;
    std::string wait;
    std::getline(std::cin, wait);
}
