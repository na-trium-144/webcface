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
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/common/val_adaptor.h"

WEBCFACE_NS_BEGIN

/*!
 * \brief 文字列、数値などの型を送受信するクラス
 * \since ver2.0
 *
 * * ver1.11までTextにあった機能をVariantとTextに分離し、TextとInputRefのベースになるクラス
 * * Textと同じメッセージで送受信するが、数値型も受け付ける
 *
 */
class WEBCFACE_DLL Variant : protected Field {
  public:
    Variant() = default;
    Variant(const Field &base);
    Variant(const Field &base, const SharedString &field)
        : Variant(Field{base, field}) {}

    friend class InputRef;
    friend struct InputRefState;
    friend class TemporalViewComponent;

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \param callback Variant型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Variant &
    onChange(std::function<void WEBCFACE_CALL_FP(Variant)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Variant &onChange(F callback) const {
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
    [[deprecated]]
    void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

  protected:
    /*!
     * \brief 値をセットする
     * \since ver1.10
     */
    const Variant &set(const ValAdaptor &v) const;

  public:
    /*!
     * \brief 値をリクエストする
     *
     */
    const Variant &request() const;
    /*!
     * \brief 値を取得する
     *
     */
    std::optional<ValAdaptor> tryGet() const;
    /*!
     * \brief 値を取得する
     *
     * <del>参照は少なくとも次のClient::sync()までは有効</del>
     * ver2.10〜 ValAdaptorをコピーで返すように変更
     *
     */
    ValAdaptor get() const;

    template <typename T,
              typename std::enable_if_t<std::is_convertible_v<ValAdaptor, T>,
                                        std::nullptr_t> = nullptr>
    operator T() const {
        return static_cast<T>(get());
    }
    /*!
     * \brief 値が空かどうか調べる
     *
     */
    bool empty() const { return get().empty(); }
    /*!
     * \brief 文字列として返す
     *
     * <del>std::stringのconst参照を返す。参照は少なくとも次のClient::sync()までは有効</del>
     *
     * \deprecated ver2.10〜
     * 内部の仕様変更によりconst参照ではなく文字列のコピーを返す。
     * コピーなしで文字列を参照するには asStringView() を使用すること。
     */
    [[deprecated("(ver2.10〜) use asStringView() or asString() instead")]]
    std::string asStringRef() const {
        return asString();
    }
    /*!
     * \brief 文字列として返す (wstring)
     * \sa asStringRef()
     * \deprecated ver2.10〜
     * 内部の仕様変更によりconst参照ではなく文字列のコピーを返す。
     * コピーなしで文字列を参照するには asStringView() を使用すること。
     */
    [[deprecated("(ver2.10〜) use asWStringView() or asWString() instead")]]
    std::wstring asWStringRef() const {
        return asWString();
    }
    /*!
     * \brief null終端の文字列の参照として返す
     * \since ver2.10
     */
    StringView asStringView() const { return get().asStringView(); }
    /*!
     * \brief null終端の文字列の参照として返す (wstring)
     * \since ver2.10
     */
    WStringView asWStringView() const { return get().asWStringView(); }
    /*!
     * \brief 文字列として返す(コピー)
     *
     */
    std::string asString() const { return get().asString(); }
    /*!
     * \brief 文字列として返す(コピー) (wstring)
     *
     */
    std::wstring asWString() const { return get().asWString(); }
    /*!
     * \brief 実数として返す
     *
     */
    double asDouble() const { return get().asDouble(); }
    /*!
     * \brief int型の整数として返す
     *
     */
    int asInt() const { return get().asInt(); }
    /*!
     * \brief long long型の整数として返す
     *
     */
    long long asLLong() const { return get().asLLong(); }
    /*!
     * \brief bool値を返す
     *
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

    template <typename T, typename std::enable_if_t<std::is_same_v<T, Variant>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Variant>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
    bool operator<(const Variant &) const = delete;
    bool operator<=(const Variant &) const = delete;
    bool operator>(const Variant &) const = delete;
    bool operator>=(const Variant &) const = delete;
};


/*!
 * \brief 文字列の送受信データを表すクラス
 *
 * コンストラクタではなく Member::text() を使って取得してください
 *
 */
class WEBCFACE_DLL Text : protected Variant {
  public:
    Text() = default;
    Text(const Field &base) : Variant(base) {}
    Text(const Field &base, const SharedString &field)
        : Text(Field{base, field}) {}

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
    Text child(StringInitializer field) const {
        return this->Field::child(static_cast<SharedString &>(field));
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Text child(int index) const {
        return this->Field::child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     *
     * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     *
     */
    Text operator[](StringInitializer field) const {
        return child(std::move(field));
    }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     *
     * Variantがlongに変換可能なため。
     *
     */
    Text operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Text operator[](const wchar_t *field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver2.10
     */
    template <typename CharT, std::size_t N,
              typename std::enable_if_t<std::is_same_v<CharT, char> ||
                                            std::is_same_v<CharT, wchar_t>,
                                        std::nullptr_t> = nullptr>
    Text operator[](const CharT (&static_str)[N]) {
        return child(StringInitializer(static_str));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Text
    operator[](int index) const {
        return child(std::to_string(index));
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Text parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback Text型の引数(thisが渡される)を1つ取る関数
     *
     */
    template <typename F,
              typename std::enable_if_t<std::is_invocable_v<F, Text>,
                                        std::nullptr_t> = nullptr>
    const Text &onChange(F callback) const {
        this->Variant::onChange(
            [callback = std::move(callback)](const Variant &base) {
                callback(Text(base));
            });
        return *this;
    }
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Text &onChange(F callback) const {
        this->Variant::onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
        return *this;
    }
    /*!
     * \deprecated
     * ver1.11まではEventTarget::appendListener()でコールバックを追加できたが、
     * ver2.0からコールバックは1個のみになった。
     * 互換性のため残しているがonChange()と同じ
     *
     */
    template <typename T>
    [[deprecated]]
    void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief 文字列をセットする
     *
     * (ver2.0からstd::stringをstd::string_viewに変更)
     * (ver2.0からstd::wstring対応、ver2.10からString型に変更)
     */
    const Text &set(StringInitializer v) const {
        this->Variant::set(ValAdaptor{std::move(v)});
        return *this;
    }

    /*!
     * \brief 文字列をセットする
     *
     * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     *
     */
    const Text &operator=(StringInitializer v) const {
        this->set(std::move(v));
        return *this;
    }

    /*!
     * \brief 文字列をリクエストする
     * \since ver1.7
     *
     */
    const Text &request() const {
        this->Variant::request();
        return *this;
    }
    /*!
     * \brief 文字列を返す
     *
     * * <del>ver1.10〜 文字列以外の型も扱うためValAdaptor型に変更</del>
     * * <del>ver2.0〜 stringに戻した</del>
     * * <del>参照は少なくとも次のClient::sync()までは有効</del>
     * * ver2.10〜 StringViewに変更
     *
     */
    std::optional<StringView> tryGet() const;
    /*!
     * \brief 文字列を返す (wstring)
     * \since ver2.0
     *
     * * <del>参照は少なくとも次のClient::sync()までは有効</del>
     * * ver2.10〜 WStringViewに変更
     *
     */
    std::optional<WStringView> tryGetW() const;
    /*!
     * \brief 文字列を返す (const参照)
     *
     * * <del>ver1.10〜 文字列以外の型も扱うためValAdaptor型に変更</del>
     * * <del>ver2.0〜 stringに戻した</del>
     * * <del>ver2.0〜 const参照に変更</del>
     * * <del>参照は少なくとも次のClient::sync()までは有効</del>
     * * ver2.10〜 StringViewに変更
     *
     */
    StringView get() const;
    /*!
     * \brief 文字列を返す (wstring const参照)
     * \since ver2.0
     *
     * * <del>参照は少なくとも次のClient::sync()までは有効</del>
     * * ver2.10〜 WStringViewに変更
     *
     */
    WStringView getW() const;

    /*!
     * \since ver2.10
     *
     * 以前の const std::string& を置き換え
     */
    operator std::string_view() const { return get(); }
    /*!
     * \since ver2.10
     *
     * 以前の const std::wstring& を置き換え
     */
    operator std::wstring_view() const { return getW(); }

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
    [[deprecated]]
    std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    const Text &free() const;

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

namespace internal {
struct InputRefState;
}

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
    std::shared_ptr<internal::InputRefState> state;

    void lockTo(const Variant &target);
    Variant &lockedField() const;

  public:
    friend class TemporalViewComponent;

    InputRef();
    InputRef(const InputRef &) = default;
    InputRef &operator=(const InputRef &) = default;
    /*!
     * moveするとfieldがnullptrになってしまうがそれはまずいのでムーブ禁止
     *
     */
    InputRef(InputRef &&) = delete;
    InputRef &operator=(InputRef &&) = delete;
    ~InputRef() = default;

    /*!
     * \brief 値を返す
     *
     * * <del>ver1.11からconst参照</del>
     *   * <del>参照は次に値を取得して別の値が返ったときまで有効</del>
     *   * <del>ver2.0〜: 参照は少なくとも次のClient::sync()までは有効</del>
     * * ver2.10〜 ValAdaptorをコピーで返すように変更
     *
     */
    ValAdaptor get() const;

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
     * * <del>std::stringのconst参照を返す。</del>
     *   * <del>参照は次に値を取得して別の値が返ったときまで有効</del>
     *   * ver2.0〜: 参照は少なくとも次のClient::sync()までは有効
     *
     * \deprecated ver2.10〜
     * 内部の仕様変更によりconst参照ではなく文字列のコピーを返す。
     * コピーなしで文字列を参照するには asStringView()
     * を使用すること。
     */
    [[deprecated("(ver2.10〜) use asStringView() or asString() instead")]]
    std::string asStringRef() const {
        return asString();
    }
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.0
     * \sa asStringRef()
     * \deprecated ver2.10〜
     * 内部の仕様変更によりconst参照ではなく文字列のコピーを返す。
     * コピーなしで文字列を参照するには asWStringView()
     * を使用すること。
     */
    [[deprecated("(ver2.10〜) use asWStringView() or asWString() instead")]]
    std::wstring asWStringRef() const {
        return asWString();
    }
    /*!
     * \brief null終端の文字列として返す
     * \since ver2.10
     */
    StringView asStringView() const { return get().asStringView(); }
    /*!
     * \brief null終端の文字列として返す
     * \since ver2.10
     */
    WStringView asWStringView() const { return get().asWStringView(); }
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
    [[deprecated("use asDouble(), asInt() or asLLong() instead")]]
    double as() const {
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
