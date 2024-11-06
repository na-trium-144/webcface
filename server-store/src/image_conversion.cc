#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include "webcface/server/server.h"
#include "webcface/message/image.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <Magick++.h>

WEBCFACE_NS_BEGIN
namespace server {

// メインのスレッドで呼ばれるべき (server側でチェック)
void initMagick() {
    static std::mutex m;
    std::lock_guard lock(m);
    static bool initialized = false;
    if (!initialized) {
        Magick::InitializeMagick(nullptr);
        initialized = true;
    }
}

static std::string magickColorMap(ImageColorMode mode) {
    switch (mode) {
    case ImageColorMode::gray:
        return "K";
    case ImageColorMode::bgr:
        return "BGR";
    case ImageColorMode::bgra:
        return "BGRA";
    case ImageColorMode::rgb:
        return "RGB";
    case ImageColorMode::rgba:
        return "RGBA";
    default:
        return "";
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
                        logger->trace("converting image of {}, {}",
                                      member.decode(), field.decode());
                        img = cd->image[field];
                    }
                    if (img.data_->size() == 0) {
                        break;
                    }


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
                        std::string color_map_before =
                            magickColorMap(img.color_mode_);
                        if (color_map_before.empty()) {
                            this->logger->error(
                                "Unknown image color mode in original image: "
                                "{}",
                                static_cast<int>(img.color_mode_));
                            return;
                        }
                        Magick::Image m(img.width_, img.height_,
                                        color_map_before, Magick::CharPixel,
                                        img.data_->data());
#ifdef WEBCFACE_MAGICK_VER7
                        // ImageMagick6と7で名前が異なる
                        m.type(Magick::TrueColorAlphaType);
#else
                        m.type(Magick::TrueColorMatteType);
#endif
                        if (img.color_mode_ == ImageColorMode::gray) {
                            // K -> RGB
                            m.negate(true);
                        }

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
                            m.resize(Magick::Geometry(cols, rows));
                        }

                        auto color_mode =
                            info.color_mode.value_or(img.color_mode_);
                        auto encoded =
                            std::make_shared<std::vector<unsigned char>>();
                        switch (info.cmp_mode) {
                        case ImageCompressMode::raw: {
                            std::size_t channels = 1;
                            std::string color_map = magickColorMap(color_mode);
                            switch (color_mode) {
                            case ImageColorMode::gray:
                                m.type(Magick::GrayscaleType);
                                color_map = "R";
                                channels = 1;
                                break;
                            case ImageColorMode::bgr:
                            case ImageColorMode::rgb:
                                channels = 3;
                                break;
                            case ImageColorMode::bgra:
                            case ImageColorMode::rgba:
                                channels = 4;
                                break;
                            default:
                                this->logger->error(
                                    "Unknown image color mode requested: {}",
                                    static_cast<int>(color_mode));
                                return;
                            }
                            encoded->resize(
                                static_cast<std::size_t>(cols * rows) *
                                channels);
                            m.write(0, 0, cols, rows, color_map,
                                    Magick::CharPixel, encoded->data());
                            break;
                        }
                        case ImageCompressMode::jpeg: {
                            if (info.quality < 0 || info.quality > 100) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(jpeg, quality={})",
                                    info.quality);
                                return;
                            }
                            m.magick("JPEG");
                            m.quality(info.quality);
                            Magick::Blob b;
                            m.write(&b);
                            encoded->assign(
                                static_cast<const unsigned char *>(b.data()),
                                static_cast<const unsigned char *>(b.data()) +
                                    b.length());
                            break;
                        }
                        case ImageCompressMode::webp: {
                            if (info.quality < 1 || info.quality > 100) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(webp, quality={})",
                                    info.quality);
                                return;
                            }
                            m.magick("WEBP");
                            m.quality(info.quality);
                            Magick::Blob b;
                            m.write(&b);
                            encoded->assign(
                                static_cast<const unsigned char *>(b.data()),
                                static_cast<const unsigned char *>(b.data()) +
                                    b.length());
                            break;
                        }
                        case ImageCompressMode::png: {
                            if (info.quality < 0 || info.quality > 100) {
                                this->logger->error(
                                    "Invalid image conversion request "
                                    "(png, compression={})",
                                    info.quality);
                                return;
                            }
                            m.magick("PNG");
                            m.quality(info.quality);
                            Magick::Blob b;
                            m.write(&b);
                            encoded->assign(
                                static_cast<const unsigned char *>(b.data()),
                                static_cast<const unsigned char *>(b.data()) +
                                    b.length());
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
}

} // namespace server
WEBCFACE_NS_END
