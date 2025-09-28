#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include <utility>
#include <stdexcept>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "robot_model.h"
#include "components.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename Component>
class DataSetBuffer;
}

/*!
 * \brief Canvas3Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas3D() を使って取得してください
 *
 */
class WEBCFACE_DLL Canvas3D : protected Field {
    std::shared_ptr<internal::DataSetBuffer<TemporalCanvas3DComponent>> sb;

  public:
    Canvas3D();
    Canvas3D(const Field &base);
    Canvas3D(const Field &base, const SharedString &field)
        : Canvas3D(Field{base, field}) {}

    friend internal::DataSetBuffer<Canvas3DComponent>;
    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     * 
     */
    Canvas3D child(StringInitializer field) const {
        return this->Field::child(static_cast<SharedString &>(field));
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Canvas3D child(int index) const {
        return this->Field::child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     * 
     * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     * 
     */
    Canvas3D operator[](StringInitializer field) const { return child(std::move(field)); }
    /*!
     * child()と同じ
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Canvas3D operator[](int index) const {
        return child(std::to_string(index));
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Canvas3D parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback Canvas3D型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Canvas3D &
    onChange(std::function<void WEBCFACE_CALL_FP(Canvas3D)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Canvas3D &onChange(F callback) const {
        return onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
    }

    /*!
     * \brief canvasの内容をリクエストする
     * \since ver1.7
     *
     */
    const Canvas3D &request() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::optional<std::vector<Canvas3DComponent>> tryGet() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::vector<Canvas3DComponent> get() const {
        return tryGet().value_or(std::vector<Canvas3DComponent>{});
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
     * \brief 値やリクエスト状態をクリア
     *
     */
    const Canvas3D &free() const;

    /*!
     * \brief このCanvas3Dに追加した内容を初期化する
     *
     * このCanvas3Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas3Dが送信される)
     *
     */
    const Canvas3D &init() const;

    /*!
     * \brief Componentを追加
     * \since ver1.9
     */
    const Canvas3D &operator<<(TemporalCanvas3DComponent cc) const;

    /*!
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     * \since ver1.9〜
     *
     */
    template <typename T>
    const Canvas3D &add(T &&cc) const {
        *this << std::forward<T>(cc);
        return *this;
    }

    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    template <bool V, bool C2>
    const Canvas3D &operator<<(TemporalComponent<V, C2, true> cc) const {
        *this << std::move(cc.component_3d);
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * このCanvas3Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    const Canvas3D &sync() const;

    /*!
     * \brief Canvas3Dの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Canvas3D>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Canvas3D>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
