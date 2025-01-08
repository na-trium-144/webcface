#pragma once
#include <functional>
#include <ostream>
#include <optional>
#include <chrono>
#include "field.h"
#include "array_like.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct ValueShape;
}
/*!
 * \since ver2.6
 */
struct WEBCFACE_DLL ValueShape {
    std::size_t fixed_size;
    bool is_fixed;
    ValueShape(std::size_t size = 1, bool fixed = false)
        : fixed_size(size), is_fixed(fixed) {}
    ValueShape(const message::ValueShape &);
    operator message::ValueShape() const;
};

/*!
 * \brief 配列型のValueデータの一部の要素を指定するクラス
 * \since ver2.6
 */
class WEBCFACE_DLL ValueIndex : protected Field {
    std::size_t index;
    ValueIndex(const Field &base, std::size_t index)
        : Field(base), index(index) {}

  public:
    friend class Value;

    /*!
     * \brief 値をセットする
     *
     * * 事前に Value::resize() でサイズを変更しておく必要がある
     * * データがない場合、範囲外の場合は std::out_of_range
     */
    const ValueIndex &set(double v) const;
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

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Value child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Value child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.6
     */
    [[deprecated]] Value child(int index) const {
        return child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Value operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    Value operator[](std::wstring_view field) const { return child(field); }
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
     * * (ver1.11〜) <del>child()と同じ</del>
     * * (ver2.6〜)
     * 配列型Valueデータの要素をset,getするためのValueIndexクラスを返す
     *   * Field::operator[] や他の型の operator[] (すべてver2.6でdeprecated)
     * とは異なる挙動になる
     */
    ValueIndex operator[](std::size_t index) const {
        return ValueIndex(*this, index);
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
    [[deprecated]] void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

  protected:
    /*!
     * \since ver2.6
     */
    const Value &set(std::vector<double> v, ValueShape shape) const;

  public:
    /*!
     * \brief 値をセットする
     *
     * * (ver1.11〜)
     * <del>vが配列でなく、parent()の配列データが利用可能ならその要素をセットする</del>
     *   * (ver2.6〜) 配列データの要素にアクセスするには、 operator[] で得られる
     * ValueIndex を使う
     * * (ver2.6〜) セットしたデータはサイズ1の固定長データとなる
     * (ValueFixed<1>()::set() と同じ)
     *
     */
    const Value &set(double v) const;
    /*!
     * \brief vector型配列をセットする
     * \since ver2.0 (set(VectorOpt<double>) を置き換え)
     *
     * * (ver2.6〜) データ型は可変長となる
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
     * * (ver2.6〜) ネストした配列も可
     * * (ver2.6〜) データ型は可変長となる
     *
     */
    template <typename R, typename traits::NestedArrayLikeTrait<R>::ArrayLike =
                              traits::TraitOk>
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
     * * データが配列だった場合、最初の要素を返す
     * * (ver1.11〜) <del>parent()の配列データが利用可能ならその要素を返す</del>
     *   * (ver2.6〜) 配列データの要素にアクセスするには、 operator[] で得られる
     * ValueIndex を使う
     *
     */
    std::optional<double> tryGet() const;
    /*!
     * \brief 値をvectorで返す
     *
     * * データが配列でない場合、サイズ1のvectorとして返す
     *
     */
    std::optional<std::vector<double>> tryGetVec() const;
    /*!
     * \brief 値を返す
     *
     * * データが配列だった場合、最初の要素を返す
     * * (ver1.11〜) <del>parent()の配列データが利用可能ならその要素を返す</del>
     *   * (ver2.6〜) 配列データの要素にアクセスするには、 operator[] で得られる
     * ValueIndex を使う
     *
     */
    double get() const { return tryGet().value_or(0); }
    /*!
     * \brief 値をvectorで返す
     *
     * * データが存在しない場合、サイズ0のvectorとして返す
     * * データが配列でない場合、サイズ1のvectorとして返す
     *
     */
    std::vector<double> getVec() const {
        return tryGetVec().value_or(std::vector<double>{});
    }
    operator double() const { return get(); }
    operator std::vector<double>() const { return getVec(); }

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
    [[deprecated]] std::chrono::system_clock::time_point time() const;

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
 * \brief 可変長配列を送受信するクラス
 * \since ver2.6
 *
 * データは Value クラスと共通のフォーマットだが、
 * データが固定長であるという情報が送信されるのに加え、
 * テンプレートで指定された配列サイズと一致しているかどうか、
 * 送られてきたデータも固定長であるかどうか、を
 * set() や get() でチェックする
 */
template <std::size_t... Shape>
class ValueFixed : Value {
    static constexpr std::size_t size = (1 * ... * Shape);
    static_assert(size > 0);

  public:
    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    using Value::child;
    using Value::Value;
    using Value::operator[];
    using Value::parent;

    template <typename F, typename std::enable_if_t<
                              std::is_invocable_v<F, ValueFixed<Shape...>>,
                              std::nullptr_t> = nullptr>
    const ValueFixed &onChange(F callback) const {
        this->Value::onChange(
            [callback = std::move(callback)](const Value &base) {
                callback(ValueFixed<Shape...>(base));
            });
        return *this;
    }
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const ValueFixed &onChange(F callback) const {
        this->Value::onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
        return *this;
    }
    /*!
     * \brief 値をセットする (配列サイズが1の場合のみ)
     *
     */
    template <std::size_t s = size,
              std::enable_if_t<s == size && s == 1, std::nullptr_t> = nullptr>
    const ValueFixed &set(double v) const {
        this->Value::set(v);
        return *this;
    }
    /*!
     * \brief vector型配列をセットする
     *
     * サイズが一致しない場合 std::invalid_argument を投げる
     */
    const ValueFixed &set(std::vector<double> v) const {
        this->Value::set(std::move(v), ValueShape{size, true});
        return *this;
    }
    /*!
     * \brief 配列型の値をセットする
     *
     * * std::begin(), std::end()
     * が使えてその値がdoubleに変換可能ならなんでもok
     * * ネストした配列も可
     * * std::array などサイズが固定の配列を渡した場合、
     * サイズが一致しなければコンパイルエラー
     * * std::vector などサイズが取得できない型を渡した場合、
     * 実行時にサイズが一致しなければ std::invalid_argument を投げる
     *
     */
    template <
        typename R,
        typename traits::NestedArrayLikeTrait<R>::ArrayLike = traits::TraitOk,
        typename traits::NestedArraySizeTrait<R, size>::SizeMatchOrDynamic =
            traits::TraitOk>
    const ValueFixed &set(const R &range) const {
        return this->set(traits::nestedArrayLikeToVector(range));
    }
    template <typename T>
    const ValueFixed &operator=(T &&v) const {
        this->set(std::forward<T>(v));
        return *this;
    }

    const ValueFixed &request() const {
        this->Value::request();
        return *this;
    }
    template <std::size_t s = size,
              std::enable_if_t<s == size && s == 1, std::nullptr_t> = nullptr>
    std::optional<double> tryGet() const {
        return this->Value::tryGet();
    }
    template <std::size_t s = size,
              std::enable_if_t<s == size && s == 1, std::nullptr_t> = nullptr>
    double get() const {
        return tryGet().value_or(0);
    }
    template <std::size_t s = size,
              std::enable_if_t<s == size && s == 1, std::nullptr_t> = nullptr>
    operator double() const {
        return get();
    }

    using Value::exists;

    const ValueFixed &free() const {
        this->Value::free();
        return *this;
    }

    template <typename T,
              typename std::enable_if_t<std::is_same_v<T, ValueFixed>,
                                        std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T,
              typename std::enable_if_t<std::is_same_v<T, ValueFixed>,
                                        std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    bool operator<(const ValueFixed &) const = delete;
    bool operator<=(const ValueFixed &) const = delete;
    bool operator>(const ValueFixed &) const = delete;
    bool operator>=(const ValueFixed &) const = delete;
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
