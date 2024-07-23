#pragma once
#include <functional>
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "field.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/config.h"
#endif
#include "webcface/encoding/val_adaptor.h"

WEBCFACE_NS_BEGIN

/*!
 * \brief 文字列の送受信データを表すクラス
 *
 * コンストラクタではなく Member::text() を使って取得してください
 *
 */
class WEBCFACE_DLL Text : protected Field {
  public:
    Text() = default;
    Text(const Field &base);
    Text(const Field &base, const SharedString &field)
        : Text(Field{base, field}) {}

    friend class InputRef;
    friend struct InputRefState;
    friend class ViewComponent;
    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Text child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Text child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Text child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Text operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    Text operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Text operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Text operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Text operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Text parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     */
    Text &onChange(std::function<void WEBCFACE_CALL_FP(Text)> callback);
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    Text &onChange(F callback) {
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
    [[deprecated]] void appendListener(T &&callback) {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief 文字列をセットする
     *
     * (ver2.0からstd::stringをstd::string_viewに変更)
     *
     */
    Text &set(std::string_view v) { return set(ValAdaptor{v}); }
    /*!
     * \brief 文字列をセットする (wstring)
     * \since ver2.0
     */
    Text &set(std::wstring_view v) { return set(ValAdaptor{v}); }
    /*!
     * \brief 文字列以外の型の値もセットする
     * \since ver1.10
     */
    Text &set(const ValAdaptor &v);

    /*!
     * \brief 文字列をセットする
     *
     */
    Text &operator=(std::string_view v) {
        this->set(v);
        return *this;
    }
    /*!
     * \brief 文字列をセットする (wstring)
     * \since ver2.0
     */
    Text &operator=(std::wstring_view v) {
        this->set(v);
        return *this;
    }

    /*!
     * \brief 文字列をリクエストする
     * \since ver1.7
     *
     */
    void request() const;
    /*!
     * \brief 文字列を返す
     *
     * * <del>ver1.10〜 文字列以外の型も扱うためValAdaptor型に変更</del>
     * * ver2.0〜 stringに戻した
     *
     */
    std::optional<std::string> tryGet() const;
    /*!
     * \brief 文字列を返す (wstring)
     * \since ver2.0
     */
    std::optional<std::wstring> tryGetW() const;
    /*!
     * \since ver2.0
     *
     * 文字列以外の型のデータが必要な場合
     *
     */
    std::optional<ValAdaptor> tryGetV() const;
    /*!
     * \brief 文字列を返す
     *
     * * <del>ver1.10〜 文字列以外の型も扱うためValAdaptor型に変更</del>
     * * ver2.0〜 stringに戻した
     *
     */
    std::string get() const { return tryGet().value_or(""); }
    /*!
     * \brief 文字列を返す (wstring)
     * \since ver2.0
     */
    std::wstring getW() const { return tryGetW().value_or(L""); }

    operator std::string() const { return get(); }
    operator std::wstring() const { return getW(); }

    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    Text &free();

    bool operator==(std::string_view rhs) const { return this->get() == rhs; }
    bool operator!=(std::string_view rhs) const { return this->get() != rhs; }
    bool operator==(std::wstring_view rhs) const { return this->getW() == rhs; }
    bool operator!=(std::wstring_view rhs) const { return this->getW() != rhs; }

    /*!
     * \brief Textの参照先を比較
     * \since ver1.11
     *
     * 1.10まではText同士を比較すると中の値が比較されていた。
     * 大小の比較も同様に中の値で比較されると非自明な挙動になるのでdeleteしている。
     *
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Text>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Text>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
    bool operator<(const Text &) const = delete;
    bool operator<=(const Text &) const = delete;
    bool operator>(const Text &) const = delete;
    bool operator>=(const Text &) const = delete;
};

/*!
 * \brief Textをostreamに渡すとTextの中身を表示
 *
 */
WEBCFACE_DLL std::ostream &WEBCFACE_CALL operator<<(std::ostream &os,
                                                    const Text &data);

struct WEBCFACE_DLL InputRefState {
    Text field;
    ValAdaptor val;
    InputRefState() = default;
};
/*!
 * \brief 名前を指定しないText
 *
 * viewでinputの値の管理に使う。
 * 数値型で用いることもあるが内部データ型としては常にTextを使用する。
 *
 * bindしたviewをsync()するときに名前が決定される(lock)。
 * InputRefオブジェクトのコピーは名前が決まる前後でつねに同じTextを参照し、
 * その後も同じInputRefオブジェクトを使用することで同じ値を参照することができる
 *
 * lockされた後にInputRefを破棄し、その後新しいInputRefを同じviewにbindした場合、
 * sync()時にその新しいInputRefには前のInputRefと同じ名前が割り当てられることで同じ値になる
 *
 */
class WEBCFACE_DLL InputRef {
    std::shared_ptr<InputRefState> state;

  public:
    InputRef() : state(std::make_shared<InputRefState>()) {}
    InputRef(const InputRef &) = default;
    InputRef &operator=(const InputRef &) = default;
    /*!
     * moveするとfieldがnullptrになってしまうがそれはまずいのでコピーをしろ。
     *
     */
    InputRef(InputRef &&) = delete;
    InputRef &operator=(InputRef &&) = delete;
    ~InputRef() = default;

    void lockTo(const Text &target) { state->field = target; }
    bool expired() const { return state->field.expired(); }
    Text &lockedField() const { return state->field; }

    /*!
     * \brief 値を返す
     *
     * ver1.11からconst参照 (次に値を取得して別の値が返ったときまで有効)
     *
     */
    const ValAdaptor &get() const {
        if (expired()) {
            if (!state->val.empty()) {
                state->val = ValAdaptor();
            }
        } else {
            auto new_val = state->field.tryGetV().value_or(ValAdaptor());
            if (state->val != new_val) {
                state->val = new_val;
            }
        }
        return state->val;
    }

    /*!
     * \brief 値を返す
     *
     */
    template <typename T,
              typename std::enable_if_t<std::is_convertible_v<ValAdaptor, T>,
                                        std::nullptr_t> = nullptr>
    operator T() const {
        return static_cast<T>(get());
    }

    /*!
     * \brief 値が空かどうか調べる
     * \since ver1.11
     */
    bool empty() const { return get().empty(); }

    /*!
     * \brief 文字列として返す
     * \since ver1.11
     *
     * std::stringのconst参照を返す。
     * 参照は次に値を取得して別の値が返ったときまで有効
     *
     */
    const std::string &asStringRef() const { return get().asStringRef(); }
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.0
     * \sa asStringRef()
     */
    const std::wstring &asWStringRef() const { return get().asWStringRef(); }
    /*!
     * \brief 文字列として返す(コピー)
     * \since ver1.11
     */
    std::string asString() const { return get().asString(); }
    /*!
     * \brief 文字列として返す(コピー) (wstring)
     * \since ver2.0
     */
    std::wstring asWString() const { return get().asWString(); }
    /*!
     * \brief 実数として返す
     * \since ver2.0
     */
    double asDouble() const { return get().asDouble(); }
    /*!
     * \brief int型の整数として返す
     * \since ver2.0
     */
    int asInt() const { return get().asInt(); }
    /*!
     * \brief long long型の整数として返す
     * \since ver2.0
     */
    long long asLLong() const { return get().asLLong(); }
    /*!
     * \brief 数値として返す
     * \since ver1.11
     *
     * as<T>(), Tはdoubleなどの実数型、intなどの整数型
     *
     * \deprecated ver2.0〜 asDouble(), asInt(), asLLong() を追加
     * さらにas<T>にはTになにを指定してもdoubleで返るというバグがある
     *
     */
    template <typename T>
    [[deprecated("use asDouble(), asInt() or asLLong() instead")]] double
    as() const {
        return get().as<T>();
    }

    /*!
     * \brief bool値を返す
     * \since ver1.11
     */
    bool asBool() const { return get().asBool(); }

    template <typename T,
              typename std::enable_if_t<std::is_constructible_v<ValAdaptor, T>,
                                        std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return get() == other;
    }
    template <typename T,
              typename std::enable_if_t<std::is_constructible_v<ValAdaptor, T>,
                                        std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return get() != other;
    }
};

template <typename T,
          typename std::enable_if_t<std::is_constructible_v<ValAdaptor, T>,
                                    std::nullptr_t> = nullptr>
bool operator==(const T &other, const InputRef &ref) {
    return ref.get() == other;
}
template <typename T,
          typename std::enable_if_t<std::is_constructible_v<ValAdaptor, T>,
                                    std::nullptr_t> = nullptr>
bool operator!=(const T &other, const InputRef &ref) {
    return ref.get() != other;
}
inline std::ostream &operator<<(std::ostream &os, const InputRef &ref) {
    return os << ref.get();
}
WEBCFACE_NS_END
