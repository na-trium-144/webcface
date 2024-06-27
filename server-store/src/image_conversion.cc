#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include <webcface/server/server.h>
#include "webcface/message/message.h"
#include <webcface/common/def.h>
#include <Magick++.h>

WEBCFACE_NS_BEGIN
namespace Server {

static void initMagick() {
    static bool initialized = false;
    if(!initialized){
        Magick::InitializeMagick(nullptr);
        initialized = true;
    }
}

static std::string magickColorMap(int mode) {
    switch (mode) {
    case 0: // ImageColorMode::gray:
        return "K";
    case 1: // ImageColorMode::bgr:
        return "BGR";
    case 2: // ImageColorMode::rgb:
        return "RGB";
    case 3: // ImageColorMode::bgra:
        return "BGRA";
    case 4: // ImageColorMode::rgba:
        return "RGBA";
    }
    return "";
}
/*!
 * \brief cdの画像を変換しthisに送信
 *
 * cd.image[field]が更新されるかリクエストが更新されたときに変換を行う。
 *
 */
void MemberData::imageConvertThreadMain(const SharedString &member,
                                        const SharedString &field) {
    initMagick();

    int last_image_flag = -1, last_req_flag = -1;
    logger->trace("imageConvertThreadMain started for {}, {}", member.decode(),
                  field.decode());
    while (true) {
        store->findAndDo(
            member,
            [&](auto cd) {
                while (!cd->closing.load() && !this->closing.load()) {
                    Message::ImageFrame img;
                    {
                        std::unique_lock lock(cd->image_m[field]);
                        cd->image_cv[field].wait_for(
                            lock, std::chrono::milliseconds(1));
                        if (cd->closing.load() || this->closing.load()) {
                            break;
                        }
                        if (cd->image_changed[field] == last_image_flag &&
                            this->image_req_changed[member][field] ==
                                last_req_flag) {
                            continue;
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

                    Magick::Image m(img.width_, img.height_,
                                    magickColorMap(img.color_mode_),
                                    Magick::CharPixel, img.data_->data());
#ifdef WEBCFACE_MAGICK_VER7
                    // ImageMagick6と7で名前が異なる
                    m.type(Magick::TrueColorAlphaType);
#else
                    m.type(Magick::TrueColorMatteType);
#endif
                    if (img.color_mode_ == 0 /* gray */) {
                        // K -> RGB
                        m.negate(true);
                    }

                    auto last_frame = std::chrono::steady_clock::now();
                    // 変換処理
                    auto info = this->image_req_info[member][field];
                    // clang-tidyの偽陽性への対処のため構造化束縛しない
                    auto req_field =
                        findReqField(this->image_req, member, field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    auto sync = webcface::Message::Sync{cd->member_id,
                                                        cd->last_sync_time};

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

                    auto color_mode = info.color_mode.value_or(img.color_mode_);
                    auto encoded =
                        std::make_shared<std::vector<unsigned char>>();
                    switch (info.cmp_mode) {
                    case 0: { // ImageCompressMode::raw: {
                        std::size_t channels = 1;
                        std::string color_map = magickColorMap(color_mode);
                        switch (color_mode) {
                        case 0: // ImageColorMode::gray:
                            m.type(Magick::GrayscaleType);
                            color_map = "R";
                            channels = 1;
                            break;
                        case 1: // ImageColorMode::bgr:
                        case 3: // ImageColorMode::rgb:
                            channels = 3;
                            break;
                        case 2: // ImageColorMode::bgra:
                        case 4: // ImageColorMode::rgba:
                            channels = 4;
                            break;
                        }
                        encoded->resize(static_cast<std::size_t>(cols * rows) *
                                        channels);
                        m.write(0, 0, cols, rows, color_map, Magick::CharPixel,
                                encoded->data());
                        break;
                    }
                    case 1: { // ImageCompressMode::jpeg: {
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
                    case 2: { // ImageCompressMode::webp: {
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
                    case 3: { // ImageCompressMode::png: {
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
                    }
                    Message::ImageFrame img_send{
                        static_cast<size_t>(cols), static_cast<size_t>(rows),
                        encoded, color_mode, info.cmp_mode};
                    logger->trace("finished converting image of {}, {}",
                                  member.decode(), field.decode());
                    if (!cd->closing.load() && !this->closing.load()) {
                        std::lock_guard lock(store->server->server_mtx);
                        this->pack(sync);
                        this->pack(Message::Res<webcface::Message::Image>{
                            req_id, sub_field, img_send});
                        logger->trace("send image_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                        this->send();
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

}
WEBCFACE_NS_END