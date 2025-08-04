#pragma once
#include <functional>
#include <ostream>
#include <optional>
#include <chrono>
#include "field.h"
#include "array_like.h"
#include "webcface/common/num_vector.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief 配列型のValueデータの一部の要素を指定するクラス
 * \since ver2.8
 */
class WEBCFACE_DLL ValueElementRef : protected Field {
    std::size_t index;

  public:
    ValueElementRef(const Field &base, std::size_t index)
        : Field(base), index(index) {}

    /*!
     * \brief 値をセットする
     *
     * * 事前に Value::resize() でサイズを変更しておく必要がある
     * * データがない場合、範囲外の場合は std::out_of_range を投げる
     *   * ver2.7以前は Value[i].set()
     * で自動的にリサイズされていたので、異なる挙動になる
     */
    const ValueElementRef &set(double v) const;
    const ValueElementRef &operator=(double v) const { return set(v); }

    /*!
     * \brief 値があればその要素を返す
     *
     * * データがない場合、範囲外の場合はstd::nullopt
     */
    std::optional<double> tryGet() const;
    /*!
     * \brief 値があればその要素を返す
     *
     * * データがない場合、範囲外の場合は0
     *
     */
    double get() const { return tryGet().value_or(0); }
};

/*!
 * \brief 実数値またはその配列の送受信データを表すクラス
 *
 * コンストラクタではなく Member::value(), Member::values(),
 * Member::onValueEntry() を使って取得してください
 *
 */
class WEBCFACE_DLL Value : protected Field {
  public:
    Value() = default;
    Value(const Field &base);
    Value(const Field &base, const SharedString &field)
        : Value(Field{base, field}) {}

    friend class ValueElementRef;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     *
     */
    Value child(String field) const {
        return this->Field::child(static_cast<SharedString &>(field));
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Value child(int index) const {
        return this->Field::child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     *
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     *
     */
    Value operator[](String field) const { return child(std::move(field)); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Value operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Value operator[](const wchar_t *field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver2.10
     */
    template <std::size_t N>
    Value operator[](const char (&static_str)[N]) {
        return child(String(static_str));
    }
    /*!
     * operator[](long, const wchar_t *)と解釈されるのを防ぐための定義
     * \since ver2.10
     */
    template <std::size_t N>
    Value operator[](const wchar_t (&static_str)[N]) {
        return child(String(static_str));
    }
    /*!
     * \brief 1次元配列型データの要素を参照する
     * \since ver2.8
     *
     * * ver1.11〜2.7では operator[](int)
     * は引数を文字列に変換したchildを返していた
     * * Field::operator[] や他の型の operator[] (すべてver2.6でdeprecated)
     * とは異なる挙動になる
     *
     * \sa at()
     */
    template <typename T,
              std::enable_if_t<std::is_integral_v<T>, std::nullptr_t> = nullptr>
    ValueElementRef operator[](T index) const {
        return ValueElementRef(*this, index);
    }
    /*!
     * \brief 1次元配列型データの要素を参照する
     * \since ver2.8
     */
    ValueElementRef at(std::size_t index) const {
        return ValueElementRef(*this, index);
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Value parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback Value型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Value &
    onChange(std::function<void WEBCFACE_CALL_FP(Value)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Value &onChange(F callback) const {
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

    /*!
     * \brief 値をセットする
     *
     * ver1.11〜:
     * vが配列でなく、parent()の配列データが利用可能ならその要素をセットする
     *
     */
    const Value &set(double v) const;
    /*!
     * \brief vector型配列をセットする
     * \since ver2.0 (set(VectorOpt<double>) を置き換え)
     *
     */
    const Value &set(std::vector<double> v) const;
    /*!
     * \brief 配列型の値をセットする
     * \since ver1.7 (set(VectorOpt(std::vector<T>)) を置き換え)
     *
     * * <del>R::value_type がdoubleに変換可能な型Rならなんでもok</del>
     * * (ver2.5〜) std::begin(), std::end()
     * が使えてその値がdoubleに変換可能ならなんでもok
     *
     */
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk>
    const Value &set(const R &range) const {
        return set(traits::arrayLikeToVector(range));
    }
    /*!
     * \brief 配列をセット、またはすでにsetされていればリサイズする
     * \since ver1.11
     */
    const Value &resize(std::size_t size) const;
    /*!
     * \brief 値をセット、またはすでに配列がsetされていれば末尾に追加
     */
    const Value &push_back(double v) const;
    /*!
     * \brief 配列データのサイズを取得
     * \since ver2.8
     *
     * * 自身のデータの場合、現在setされているデータのサイズ、またはセットされていなければ0
     * * 他のmemberのデータの場合、すでに受信したデータのサイズ
     *   *
     * 受信していない場合は、get()やtryGet()と同様にリクエストを送り、0を返す
     * * 配列でない数値データ1つの場合、1を返す
     */
    std::size_t size() const;

    /*!
     * \brief 数値または配列をセットする
     *
     */
    template <typename T>
    const Value &operator=(T &&v) const {
        this->set(std::forward<T>(v));
        return *this;
    }
    /*!
     * \brief vector型配列をセットする
     * \since ver2.0.2
     *
     * initializer_listを受け取るためのオーバーロード
     * (ver1.11まで VectorOpt<double> として受け取っていたもの)
     *
     */
    const Value &operator=(std::vector<double> v) const {
        this->set(std::move(v));
        return *this;
    }

    /*!
     * \brief 値をリクエストする
     * \since ver1.7
     *
     */
    const Value &request() const;
    /*!
     * \brief 値を返す
     *
     * ver1.11〜 parent()の配列データが利用可能ならその要素を返す
     *
     */
    std::optional<double> tryGet() const;
    /*!
     * \brief 値をvectorで返す
     *
     * ver2.10〜 `std::vector<double>` から webcface::NumVector 型に変更
     * (NumVectorはvectorのconst参照にキャスト可能)
     *
     */
    std::optional<NumVector> tryGetVec() const;
    /*!
     * \brief 値を返す
     *
     */
    double get() const { return tryGet().value_or(0); }
    /*!
     * \brief 値をvectorで返す
     *
     * ver2.10〜 `std::vector<double>` から webcface::NumVector 型に変更
     * (NumVectorはvectorのconst参照にキャスト可能)
     *
     */
    NumVector getVec() const {
        return tryGetVec().value_or(std::vector<double>{});
    }
    operator double() const { return get(); }
    operator std::vector<double>() const { return getVec(); }
    /*!
     * \since ver2.10
     */
    operator NumVector() const { return getVec(); }

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
     * \deprecated 1.7で Member::syncTime() に変更
     */
    [[deprecated]]
    std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    const Value &free() const;

    const Value &operator+=(double rhs) const {
        this->set(this->get() + rhs);
        return *this;
    }
    const Value &operator-=(double rhs) const {
        this->set(this->get() - rhs);
        return *this;
    }
    const Value &operator*=(double rhs) const {
        this->set(this->get() * rhs);
        return *this;
    }
    const Value &operator/=(double rhs) const {
        this->set(this->get() / rhs);
        return *this;
    }
    const Value &operator%=(std::int32_t rhs) const {
        this->set(static_cast<std::int32_t>(this->get()) % rhs);
        return *this;
    }
    const Value &operator<<=(std::int32_t rhs) const {
        // todo: int64_tかuint64_tにしたほうがいいかもしれない
        this->set(static_cast<std::int32_t>(this->get()) << rhs);
        return *this;
    }
    const Value &operator>>=(std::int32_t rhs) const {
        this->set(static_cast<std::int32_t>(this->get()) >> rhs);
        return *this;
    }
    const Value &operator&=(std::int32_t rhs) const {
        this->set(static_cast<std::int32_t>(this->get()) & rhs);
        return *this;
    }
    const Value &operator|=(std::int32_t rhs) const {
        this->set(static_cast<std::int32_t>(this->get()) | rhs);
        return *this;
    }
    const Value &operator^=(std::int32_t rhs) const {
        this->set(static_cast<std::int32_t>(this->get()) ^ rhs);
        return *this;
    }
    /*!
     * \brief 1足したものをsetした後自身を返す
     *
     */
    const Value &operator++() const { // ++s
        this->set(this->get() + 1);
        return *this;
    }
    /*!
     * \brief 1足したものをsetし、足す前の値を返す
     *
     */
    double operator++(int) const { // s++
        auto v = this->get();
        this->set(v + 1);
        return v;
    }
    /*!
     * \brief 1引いたものをsetした後自身を返す
     *
     */
    const Value &operator--() const { // --const s
        this->set(this->get() - 1);
        return *this;
    }
    /*!
     * \brief 1引いたものをsetし、足す前の値を返す
     *
     */
    double operator--(int) const { // s--
        auto v = this->get();
        this->set(v - 1);
        return v;
    }

    /*!
     * \brief Valueの参照先を比較
     * \since ver1.11
     *
     * 1.10まではValue同士を比較すると中の値が比較されていた。
     * 大小の比較も同様に中の値で比較されると非自明な挙動になるのでdeleteしている。
     *
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Value>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Value>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    bool operator<(const Value &) const = delete;
    bool operator<=(const Value &) const = delete;
    bool operator>(const Value &) const = delete;
    bool operator>=(const Value &) const = delete;
};

/*!
 * \brief Valueをostreamに渡すとValueの中身を表示
 *
 * 1.8〜 複数の値に対応 & 受信してない時nullと表示するようにした
 *
 */
WEBCFACE_DLL std::ostream &WEBCFACE_CALL operator<<(std::ostream &os,
                                                    const Value &data);

WEBCFACE_NS_END
