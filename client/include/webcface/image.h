#pragma once
#include <functional>
#include <optional>
#include <chrono>
#include "image_frame.h"
#include "field.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief (ver1.3から追加) 画像の送受信データを表すクラス
 *
 * コンストラクタではなく Member::image() を使って取得してください
 */
class WEBCFACE_DLL Image : protected Field {
    const Image &request(std::optional<int> rows, std::optional<int> cols,
                         ImageCompressMode cmp_mode, int quality,
                         std::optional<ImageColorMode> color_mode,
                         std::optional<double> frame_rate) const;

  public:
    Image() = default;
    Image(const Field &base);
    Image(const Field &base, const SharedString &field)
        : Image(Field{base, field}) {}

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Image child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Image child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Image child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Image operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    Image operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Image operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Image operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Image operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Image parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback Image型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Image &
    onChange(std::function<void WEBCFACE_CALL_FP(Image)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Image &onChange(F callback) const {
        return onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
    }
    /*!
     * \deprecated
     * ver1.11まではEventTarget::appendListener()でコールバックを追加できたが、
     * ver2.0からコールバックは1個のみになった。
     * 互換性のため残しているがonChange()と同じ
     *
     */
    template <typename T>
    [[deprecated]] void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief 画像をセットする
     *
     */
    const Image &set(const ImageFrame &img) const;
    /*!
     * \brief 画像をセットする
     *
     */
    const Image &operator=(const ImageFrame &img) const {
        this->set(img);
        return *this;
    }

  protected:
    const Image &tryRequest() const;

  public:
    /*!
     * \brief 画像を生画像のフォーマットでリクエストする
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * rows,colsのどちらかのみがnulloptの場合縦横比を保ってリサイズ
     * \param color_mode 画像の色フォーマット
     * (nulloptの場合元画像のフォーマット)
     * \param frame_rate 画像を受信する頻度
     * (指定しない場合元画像が更新されるたびに受信する)
     * \deprecated ver2.0〜 rows, colsの順番がややこしいので sizeHW()
     * を使ってサイズ指定
     *
     */
    [[deprecated("Ambiguous image size")]]
    const Image &
    request(std::optional<int> rows, std::optional<int> cols = std::nullopt,
            std::optional<ImageColorMode> color_mode = std::nullopt,
            std::optional<double> frame_rate = std::nullopt) const {
        return request(rows, cols, ImageCompressMode::raw, 0, color_mode,
                       frame_rate);
    }
    /*!
     * \brief 画像を生画像のフォーマットでリクエストする
     * \since ver2.0
     * \param sizeOption 画像のサイズ (sizeWH() または sizeHW(), std::nullopt可)
     * rows,colsのどちらかのみがnulloptの場合縦横比を保ってリサイズ
     * \param color_mode 画像の色フォーマット
     * (nulloptの場合元画像のフォーマット)
     * \param frame_rate 画像を受信する頻度
     * (指定しない場合元画像が更新されるたびに受信する)
     *
     */
    const Image &
    request(std::optional<SizeOption> size = std::nullopt,
            std::optional<ImageColorMode> color_mode = std::nullopt,
            std::optional<double> frame_rate = std::nullopt) const {
        return request(size.value_or(SizeOption{}).rows(),
                       size.value_or(SizeOption{}).cols(),
                       ImageCompressMode::raw, 0, color_mode, frame_rate);
    }
    /*!
     * \brief 画像を圧縮されたフォーマットでリクエストする
     * \param rows 画像の高さ
     * \param cols 画像の幅
     * rows,colsのどちらかのみがnulloptの場合縦横比を保ってリサイズ
     * \param cmp_mode 圧縮モード
     * \param quality 圧縮のパラメータ
     * * jpeg → 0〜100 (大きいほうが高品質)
     * * png → 0〜9 (大きいほうが圧縮後のサイズが小さい)
     * * webp → 1〜100 (大きいほうが高品質)
     * \param frame_rate 画像を受信する頻度
     * (指定しない場合元画像が更新されるたびに受信する)
     * \deprecated ver2.0〜 rows, colsの順番がややこしいので sizeHW()
     * を使ってサイズ指定
     *
     */
    [[deprecated("Ambiguous image size")]]
    const Image &
    request(std::optional<int> rows, std::optional<int> cols,
            ImageCompressMode cmp_mode, int quality,
            std::optional<double> frame_rate = std::nullopt) const {
        return request(rows, cols, cmp_mode, quality, std::nullopt, frame_rate);
    }
    /*!
     * \brief 画像を圧縮されたフォーマットでリクエストする
     * \since ver2.0
     * \param sizeOption 画像のサイズ (sizeWH() または sizeHW(), std::nullopt可)
     * rows,colsのどちらかのみがnulloptの場合縦横比を保ってリサイズ
     * \param cmp_mode 圧縮モード
     * \param quality 圧縮のパラメータ
     * * jpeg → 0〜100 (大きいほうが高品質)
     * * png → 0〜9 (大きいほうが圧縮後のサイズが小さい)
     * * webp → 1〜100 (大きいほうが高品質)
     * \param frame_rate 画像を受信する頻度
     * (指定しない場合元画像が更新されるたびに受信する)
     *
     */
    const Image &
    request(std::optional<SizeOption> size, ImageCompressMode cmp_mode,
            int quality,
            std::optional<double> frame_rate = std::nullopt) const {
        return request(size.value_or(SizeOption{}).rows(),
                       size.value_or(SizeOption{}).cols(), cmp_mode, quality,
                       std::nullopt, frame_rate);
    }
    /*!
     * \brief 画像を返す
     *
     * リクエストしていない場合すべてデフォルトで(元画像のフォーマットで)リクエストする
     *
     */
    std::optional<ImageFrame> tryGet() const;
    /*!
     * \brief 画像を返す (データがない場合0x0の画像が返る)
     *
     * リクエストしていない場合すべてデフォルトで(元画像のフォーマットで)リクエストする
     *
     */
    ImageFrame get() const { return tryGet().value_or(ImageFrame{}); }

    // operator ImageFrame() const { return get(); }

    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     *
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    const Image &free() const;

    //! 画像をクリア (リクエスト状態は解除しない)
    const Image &clear() const;

    /*!
     * \brief Imageの参照先を比較
     * \since ver1.11
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Image>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Image>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};

WEBCFACE_NS_END
