#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "field.h"
#include "event_target.h"
#include <webcface/common/def.h>
#include "webcface/encoding/val_adaptor.h"

WEBCFACE_NS_BEGIN

/*!
 * \brief 文字列の送受信データを表すクラス
 *
 * コンストラクタではなく Member::text() を使って取得してください
 *
 */
class WEBCFACE_DLL Text : protected Field, public EventTarget<Text> {
    void onAppend() const override final;

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

    // 1.10でstd::stringをValAdaptorに変更したら使えなくなった
    // using Dict = Common::Dict<std::shared_ptr<ValAdaptor>>;
    // /*!
    //  * \brief Dictの値を再帰的にセットする
    //  *
    //  */
    // Text &set(const Dict &v);

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

    // /*!
    //  * \brief Dictの値を再帰的にセットする
    //  *
    //  */
    // Text &operator=(const Dict &v) {
    //     this->set(v);
    //     return *this;
    // }
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
    // /*!
    //  * \brief 文字列を再帰的に取得しDictで返す
    //  *
    //  */
    // std::optional<Dict> tryGetRecurse() const;
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

    // /*!
    //  * \brief 値を再帰的に取得しDictで返す
    //  *
    //  */
    // Dict getRecurse() const { return tryGetRecurse().value_or(Dict{}); }
    operator std::string() const { return get(); }
    operator std::wstring() const { return getW(); }
    // operator Dict() const { return getRecurse(); }

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
    bool operator==(std::wstring_view rhs) const { return this->getW() == rhs; }

    /*!
     * \brief Textの参照先を比較
     * \since ver1.11
     *
     * 1.10まではText同士を比較すると中の値が比較されていた。
     * 大小の比較も同様に中の値で比較されると非自明な挙動になるのでdeleteしている。
     *
     */
    template <typename T>
        requires std::same_as<T, Text>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
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
WEBCFACE_DLL std::ostream &operator<<(std::ostream &os, const Text &data);

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
    template <typename T>
        requires std::convertible_to<ValAdaptor, T>
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
     * \brief 数値として返す
     * \since ver1.11
     *
     * as<T>(), Tはdoubleなどの実数型、intなどの整数型
     *
     */
    template <typename T>
        requires(std::convertible_to<double, T> && !std::same_as<T, bool>)
    double as() const {
        return get().as<T>();
    }

    /*!
     * \brief bool値を返す
     * \since ver1.11
     */
    bool asBool() const { return get().asBool(); }

    template <typename T>
        requires std::constructible_from<ValAdaptor, T>
    bool operator==(const T &other) const {
        return get() == other;
    }
};

template <typename T>
    requires std::constructible_from<ValAdaptor, T>
bool operator==(const T &other, const InputRef &ref) {
    return ref.get() == other;
}
inline std::ostream &operator<<(std::ostream &os, const InputRef &ref) {
    return os << ref.get();
}
WEBCFACE_NS_END
