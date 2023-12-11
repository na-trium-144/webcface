#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/image.h"
#include "field.h"
#include "event_target.h"

namespace webcface {
namespace Internal {
struct ClientData;
}
class Member;

//! 画像の送受信データを表すクラス
/*! コンストラクタではなく Member::image() を使って取得してください
 */
class Image : protected Field, public EventTarget<Image> {
    WEBCFACE_DLL void onAppend() const override;

  public:
    Image() = default;
    WEBCFACE_DLL Image(const Field &base);
    WEBCFACE_DLL Image(const Field &base, const std::string &field)
        : Image(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 子フィールドを返す
    /*!
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするValue
     */
    Image child(const std::string &field) {
        return Image{*this, this->field_ + "." + field};
    }

    //! 画像をセットする
    WEBCFACE_DLL Image &set(const ImageData &img);
    //! 画像をセットする
    Image &operator=(const ImageData &img) {
        this->set(img);
        return *this;
    }

    //! 画像を返す
    WEBCFACE_DLL std::optional<ImageData> tryGet() const;
    //! 画像を返す (データがない場合0x0の画像が返る)
    ImageData get() const { return tryGet().value_or(ImageData{}); }

    operator ImageData() const { return get(); }

    //! syncの時刻を返す
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL Image &free();
};

} // namespace webcface
