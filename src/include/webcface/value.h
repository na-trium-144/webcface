#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/dict.h"
#include "common/vector.h"
#include "field.h"
#include "event_target.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
}
class Member;

class Value;
extern template class WEBCFACE_IMPORT EventTarget<Value>;

/*!
 * \brief 実数値またはその配列の送受信データを表すクラス
 *
 * コンストラクタではなく Member::value(), Member::values(),
 * Member::onValueEntry() を使って取得してください
 *
 */
class WEBCFACE_DLL Value : protected Field, public EventTarget<Value> {
    void onAppend() const override final;

  public:
    Value() = default;
    Value(const Field &base);
    Value(const Field &base, std::u8string_view field)
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
     * \since ver1.12
     */
    Value child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Value child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Value operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.12
     */
    Value operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Value operator[](const char *field) const { return child(field); }
    /*!
     * \since ver1.12
     */
    Value operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Value operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Value parent() const { return this->Field::parent(); }

    using Dict = Common::Dict<std::shared_ptr<Common::VectorOpt<double>>>;
    /*!
     * \brief Dictの値を再帰的にセットする
     * \deprecated ver1.11〜
     */
    [[deprecated]] Value &set(const Dict &v);
    /*!
     * \brief 数値または配列をセットする
     *
     * ver1.11〜
     * vが配列でなく、parent()の配列データが利用可能ならその要素をセットする
     *
     */
    Value &set(const VectorOpt<double> &v);
    /*!
     * \brief 配列をセット、またはすでにsetされていればリサイズする
     * \since ver1.11
     */
    Value &resize(std::size_t size);
    /*!
     * \brief 値をセット、またはすでに配列がsetされていれば末尾に追加
     */
    Value &push_back(double v);

    /*!
     * \brief Dictの値を再帰的にセットする
     * \deprecated ver1.11〜
     */
    template <typename = void>
    [[deprecated]] Value &operator=(const Dict &v) {
        this->set(v);
        return *this;
    }
    /*!
     * \brief 数値または配列をセットする
     *
     * ver1.11〜
     * vが配列でなく、parent()の配列データが利用可能ならその要素をセットする
     *
     */
    Value &operator=(const VectorOpt<double> &v) {
        this->set(v);
        return *this;
    }
    /*!
     * \brief 値をリクエストする
     * \since ver1.7
     *
     */
    void request() const;
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
     */
    std::optional<std::vector<double>> tryGetVec() const;
    /*!
     * \brief 値を再帰的に取得しDictで返す
     * \deprecated ver1.11〜
     */
    [[deprecated]] std::optional<Dict> tryGetRecurse() const;
    /*!
     * \brief 値を返す
     *
     */
    double get() const { return tryGet().value_or(0); }
    /*!
     * \brief 値をvectorで返す
     *
     */
    std::vector<double> getVec() const {
        return tryGetVec().value_or(std::vector<double>{});
    }
    /*!
     * \brief 値を再帰的に取得しDictで返す
     * \deprecated ver1.11〜
     */
    template <typename = void>
    [[deprecated]] Dict getRecurse() const {
        return tryGetRecurse().value_or(Dict{});
    }
    operator double() const { return get(); }
    operator std::vector<double>() const { return getVec(); }
    template <typename = void>
    [[deprecated]] operator Dict() const {
        return getRecurse();
    }
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7で Member::syncTime() に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    Value &free();

    Value &operator+=(double rhs) {
        this->set(this->get() + rhs);
        return *this;
    }
    Value &operator-=(double rhs) {
        this->set(this->get() - rhs);
        return *this;
    }
    Value &operator*=(double rhs) {
        this->set(this->get() * rhs);
        return *this;
    }
    Value &operator/=(double rhs) {
        this->set(this->get() / rhs);
        return *this;
    }
    Value &operator%=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) % rhs);
        return *this;
    }
    Value &operator<<=(std::int32_t rhs) {
        // todo: int64_tかuint64_tにしたほうがいいかもしれない
        this->set(static_cast<std::int32_t>(this->get()) << rhs);
        return *this;
    }
    Value &operator>>=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) >> rhs);
        return *this;
    }
    Value &operator&=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) & rhs);
        return *this;
    }
    Value &operator|=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) | rhs);
        return *this;
    }
    Value &operator^=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) ^ rhs);
        return *this;
    }
    /*!
     * \brief 1足したものをsetした後自身を返す
     *
     */
    Value &operator++() { // ++s
        this->set(this->get() + 1);
        return *this;
    }
    /*!
     * \brief 1足したものをsetし、足す前の値を返す
     *
     */
    double operator++(int) { // s++
        auto v = this->get();
        this->set(v + 1);
        return v;
    }
    /*!
     * \brief 1引いたものをsetした後自身を返す
     *
     */
    Value &operator--() { // --s
        this->set(this->get() - 1);
        return *this;
    }
    /*!
     * \brief 1引いたものをsetし、足す前の値を返す
     *
     */
    double operator--(int) { // s--
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
    template <typename T>
        requires std::same_as<T, Value>
    bool operator==(const T &other) const {
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
WEBCFACE_DLL std::ostream &operator<<(std::ostream &os, const Value &data);

WEBCFACE_NS_END
