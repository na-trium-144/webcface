#pragma once
#include <memory>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "field.h"
#include "components.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename Component>
class DataSetBuffer;
class Canvas2DDataBuf;
} // namespace internal

/*!
 * \brief Canvas2Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas2D() を使って取得してください
 *
 */
class WEBCFACE_DLL Canvas2D : protected Field {
    std::shared_ptr<internal::Canvas2DDataBuf> sb;

  public:
    Canvas2D();
    Canvas2D(const Field &base);
    Canvas2D(const Field &base, const SharedString &field)
        : Canvas2D(Field{base, field}) {}
    Canvas2D(const Field &base, const SharedString &field, double width,
             double height)
        : Canvas2D(Field{base, field}) {
        init(width, height);
    }

    friend internal::DataSetBuffer<Canvas2DComponent>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Canvas2D child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Canvas2D child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Canvas2D child(int index) const {
        return this->Field::child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Canvas2D operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    Canvas2D operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Canvas2D operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Canvas2D operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Canvas2D operator[](int index) const {
        return child(std::to_string(index));
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Canvas2D parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback Canvas2D型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Canvas2D &
    onChange(std::function<void WEBCFACE_CALL_FP(Canvas2D)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Canvas2D &onChange(F callback) const {
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
     * \brief Canvasの内容をリクエストする
     * \since ver1.7
     *
     */
    const Canvas2D &request() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::optional<std::vector<Canvas2DComponent>> tryGet() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::vector<Canvas2DComponent> get() const {
        return tryGet().value_or(std::vector<Canvas2DComponent>{});
    }
    /*!
     * \brief このフィールドにデータが存在すればtrue
     * \since ver2.1
     *
     * tryGet() などとは違って、実際のデータを受信しない。
     * リクエストも送信しない。
     *
     */
    bool exists() const;

    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    const Canvas2D &free() const;

    /*!
     * \brief Canvasのサイズを指定 & このCanvas2Dに追加した内容を初期化する
     *
     * このCanvas2Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas2Dが送信される)
     *
     */
    const Canvas2D &init(double width, double height) const;

    /*!
     * \brief Componentを追加
     * \since ver1.9
     *
     * * ver2.0〜: move専用
     *
     */
    const Canvas2D &operator<<(TemporalCanvas2DComponent cc) const;

    /*!
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     * \since ver1.9〜
     *
     */
    template <typename T>
    const Canvas2D &add(T &&cc) const {
        *this << std::forward<T>(cc);
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \since ver1.9
     *
     */
    template <bool V, bool C3>
    const Canvas2D &operator<<(TemporalComponent<V, true, C3> cc) const {
        *this << std::move(cc.component_2d);
        return *this;
    }
    /*!
     * \brief 内容をclientに反映し送信可能にする
     *
     * このCanvas2Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    const Canvas2D &sync() const;

    /*!
     * \brief Canvas2Dの参照先を比較
     * \since ver1.11
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Canvas2D>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Canvas2D>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
