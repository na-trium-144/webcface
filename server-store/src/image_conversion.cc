#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include "webcface/server/server.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <vips/vips8>

WEBCFACE_NS_BEGIN
namespace server {

// メインのスレッドで呼ばれるべき (server側でチェック)
void initVips() {
    static std::mutex m;
    std::lock_guard lock(m);
    static bool initialized = false;
    if (!initialized) {
        if (VIPS_INIT("")) {
            throw std::runtime_error("Failed to initialize libvips");
        }
        initialized = true;
    }
}

/*!
 * \brief cdの画像を変換しthisに送信
 *
 * cd.image[field]が更新されるかリクエストが更新されたときに変換を行う。
 *
 */
void MemberData::imageConvertThreadMain(const SharedString &member,
                                        const SharedString &field) {
    int last_image_flag = -1, last_req_flag = -1;
    logger->trace("imageConvertThreadMain started for {}, {}", member.decode(),
                  field.decode());
    while (true) {
        store->findAndDo(
            member,
            [&](auto cd) {
                while (!cd->closing.load() && !this->closing.load()) {
                    message::ImageFrame img;
                    {
                        std::unique_lock lock(this->image_m);
                        this->image_cv.wait(lock, [&] {
                            return cd->closing.load() || this->closing.load() ||
                                   cd->image_changed[field] !=
                                       last_image_flag ||
                                   this->image_req_changed[member][field] !=
                                       last_req_flag;
                        });
                        if (cd->closing.load() || this->closing.load()) {
                            break;
                        }
                        last_image_flag = cd->image_changed[field];
                        last_req_flag = this->image_req_changed[member][field];
                        if (cd->image.count(field)) {
                            img = cd->image[field];
                        }
                    }
                    if (img.empty()) {
                        break;
                    }
                    logger->trace("converting image of {}, {}", member.decode(),
                                  field.decode());


                    auto last_frame = std::chrono::steady_clock::now();
                    // 変換処理
                    auto info = this->image_req_info[member][field];
                    // clang-tidyの偽陽性への対処のため構造化束縛しない
                    auto req_field =
                        findReqField(this->image_req, member, field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    auto sync = webcface::message::Sync{cd->member_id,
                                                        cd->last_sync_time};
                    try {
                        vips::VImage m_original, m_rgba, m_output;
                        switch (img.color_mode_) {
                        case message::ImageColorMode::gray:
                            m_original = vips::VImage::new_from_memory(
                                img.rawPtr(), img.rawSize(), img.width_,
                                img.height_, 1, VIPS_FORMAT_UCHAR);
                            m_rgba =
                                m_original
                                    .colourspace(VIPS_INTERPRETATION_sRGB,
                                                 vips::VImage::option()->set(
                                                     "source_space",
                                                     VIPS_INTERPRETATION_B_W))
                                    .bandjoin(255);
                            break;
                        case message::ImageColorMode::bgr:
                            m_original = vips::VImage::new_from_memory(
                                img.rawPtr(), img.rawSize(), img.width_,
                                img.height_, 3, VIPS_FORMAT_UCHAR);
                            m_rgba = m_original[2]
                                         .bandjoin(m_original[1])
                                         .bandjoin(m_original[0])
                                         .bandjoin(255);
                            break;
                        case message::ImageColorMode::rgb:
                            m_original = vips::VImage::new_from_memory(
                                img.rawPtr(), img.rawSize(), img.width_,
                                img.height_, 3, VIPS_FORMAT_UCHAR);
                            m_rgba = m_original.bandjoin(255);
                            break;
                        case message::ImageColorMode::bgra:
                            m_original = vips::VImage::new_from_memory(
                                img.rawPtr(), img.rawSize(), img.width_,
                                img.height_, 4, VIPS_FORMAT_UCHAR);
                            m_rgba = m_original[2]
                                         .bandjoin(m_original[1])
                                         .bandjoin(m_original[0])
                                         .bandjoin(m_original[3]);
                            break;
                        case message::ImageColorMode::rgba:
                            m_original = vips::VImage::new_from_memory(
                                img.rawPtr(), img.rawSize(), img.width_,
                                img.height_, 4, VIPS_FORMAT_UCHAR);
                            m_rgba = m_original;
                            break;
                        default:
                            throw std::invalid_argument(
                                "Unknown image color mode in original image: " +
                                std::to_string(
                                    static_cast<int>(img.color_mode_)));
                        }
                        assert(m_rgba.bands() == 4);

                        int rows = static_cast<int>(img.height_);
                        int cols = static_cast<int>(img.width_);

                        if (info.rows || info.cols) {
                            if (info.rows) {
                                rows = *info.rows;
                            } else {
                                rows = static_cast<int>(
                                    static_cast<double>(*info.cols) *
                                    static_cast<double>(img.height_) /
                                    static_cast<double>(img.width_));
                            }
                            if (info.cols) {
                                cols = *info.cols;
                            } else {
                                cols = static_cast<int>(
                                    static_cast<double>(*info.rows) *
                                    static_cast<double>(img.width_) /
                                    static_cast<double>(img.height_));
                            }

                            if (rows <= 0 || cols <= 0) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(rows={}, cols={})",
                                    rows, cols);
                                return;
                            }
                            m_rgba = m_rgba.thumbnail_image(
                                cols,
                                vips::VImage::option()->set("width", rows));
                        }
                        assert(m_rgba.bands() == 4);

                        auto color_mode =
                            info.color_mode.value_or(img.color_mode_);
                        auto encoded =
                            std::make_shared<std::vector<unsigned char>>();
                        switch (info.cmp_mode) {
                        case message::ImageCompressMode::raw: {
                            switch (color_mode) {
                            case message::ImageColorMode::gray:
                                m_output = m_rgba.colourspace(
                                    VIPS_INTERPRETATION_B_W,
                                    vips::VImage::option()->set(
                                        "source_space",
                                        VIPS_INTERPRETATION_sRGB));
                                m_output = m_output[0];
                                assert(m_output.bands() == 1);
                                break;
                            case message::ImageColorMode::bgr:
                                m_output =
                                    m_rgba[2].bandjoin(m_rgba[1]).bandjoin(
                                        m_rgba[0]);
                                assert(m_output.bands() == 3);
                                break;
                            case message::ImageColorMode::rgb:
                                m_output =
                                    m_rgba[0].bandjoin(m_rgba[1]).bandjoin(
                                        m_rgba[2]);
                                assert(m_output.bands() == 3);
                                break;
                            case message::ImageColorMode::bgra:
                                m_output = m_rgba[2]
                                               .bandjoin(m_rgba[1])
                                               .bandjoin(m_rgba[0])
                                               .bandjoin(m_rgba[3]);
                                assert(m_output.bands() == 4);
                                break;
                            case message::ImageColorMode::rgba:
                                m_output = m_rgba[0]
                                               .bandjoin(m_rgba[1])
                                               .bandjoin(m_rgba[2])
                                               .bandjoin(m_rgba[3]);
                                assert(m_output.bands() == 4);
                                break;
                            default:
                                throw std::invalid_argument(
                                    "Unknown image color mode requested: " +
                                    std::to_string(
                                        static_cast<int>(color_mode)));
                                return;
                            }
                            std::size_t data_size;
                            unsigned char *data = static_cast<unsigned char *>(
                                m_output.write_to_memory(&data_size));
                            assert(m_output.width() == cols);
                            assert(m_output.height() == rows);
                            assert(data_size ==
                                   static_cast<std::size_t>(m_output.width() *
                                                            m_output.height() *
                                                            m_output.bands()));
                            encoded->assign(data, data + data_size);
                            g_free(data);
                            break;
                        }
                        case message::ImageCompressMode::jpeg: {
                            if (info.quality < 0 || info.quality > 100) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(jpeg, quality={})",
                                    info.quality);
                                return;
                            }
                            std::size_t data_size;
                            void *data;
                            m_rgba.write_to_buffer(
                                ".jpg", &data, &data_size,
                                vips::VImage::option()->set("Q", info.quality));
                            encoded->assign(static_cast<unsigned char *>(data),
                                            static_cast<unsigned char *>(data) +
                                                data_size);
                            g_free(data);
                            break;
                        }
                        case message::ImageCompressMode::webp: {
                            if (info.quality < 1 || info.quality > 100) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(webp, quality={})",
                                    info.quality);
                                return;
                            }
                            std::size_t data_size;
                            void *data;
                            m_rgba.write_to_buffer(
                                ".webp", &data, &data_size,
                                vips::VImage::option()->set("Q", info.quality));
                            encoded->assign(static_cast<unsigned char *>(data),
                                            static_cast<unsigned char *>(data) +
                                                data_size);
                            g_free(data);
                            break;
                        }
                        case message::ImageCompressMode::png: {
                            if (info.quality < 0 || info.quality > 100) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(png, compression={})",
                                    info.quality);
                                return;
                            }
                            std::size_t data_size;
                            void *data;
                            m_rgba.write_to_buffer(
                                ".png", &data, &data_size,
                                vips::VImage::option()->set("compression",
                                                            info.quality));
                            encoded->assign(static_cast<unsigned char *>(data),
                                            static_cast<unsigned char *>(data) +
                                                data_size);
                            g_free(data);
                            break;
                        }
                        default:
                            this->logger->error(
                                "Unknown image compression mode requested: {}",
                                static_cast<int>(info.cmp_mode));
                            return;
                        }
                        message::ImageFrame img_send{cols, rows, encoded,
                                                     color_mode, info.cmp_mode};
                        logger->trace("finished converting image of {}, {}",
                                      member.decode(), field.decode());
                        if (!cd->closing.load() && !this->closing.load()) {
                            std::lock_guard lock(store->server->server_mtx);
                            this->pack(sync);
                            this->pack(message::Res<webcface::message::Image>{
                                req_id, sub_field, img_send});
                            logger->trace("send image_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                            this->send();
                        }
                    } catch (const std::exception &e) {
                        logger->error("Failed to convert image: {}", e.what());
                    }
                    if (info.frame_rate && *info.frame_rate > 0) {
                        std::chrono::milliseconds delay{
                            static_cast<int>(1000 / *info.frame_rate)};
                        while (std::chrono::duration_cast<
                                   std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() -
                                   last_frame) < delay &&
                               !cd->closing.load() && !this->closing.load()) {
                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(1));
                        }
                        // last_frame = std::chrono::steady_clock::now();
                    }
                }
            },
            [] { std::chrono::milliseconds(1); });
        if (this->closing.load()) {
            break;
        }
    }
    logger->trace("imageConvertThreadMain for {}, {} stopped", member.decode(),
                  field.decode());
}

} // namespace server
WEBCFACE_NS_END
