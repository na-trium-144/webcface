#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/dict.h"
#include "common/vector.h"
#include "field.h"
#include "event_target.h"

namespace WebCFace {
namespace Internal {
struct ClientData;
}
class Member;

//! 実数値またはその配列の送受信データを表すクラス
/*! コンストラクタではなく Member::value(), Member::values(),
 * Member::onValueEntry() を使って取得してください
 */
class Value : protected Field, public EventTarget<Value> {
public:
    using Dict = Common::Dict<std::shared_ptr<Common::VectorOpt<double>>>;
    private:

    WEBCFACE_DLL Value &set(const std::shared_ptr<VectorOpt<double>> &v);
    WEBCFACE_DLL std::optional<std::shared_ptr<VectorOpt<double>>> getRaw() const;
    WEBCFACE_DLL std::optional<Dict> getRawRecurse() const;

    void onAppend() const override { getRaw(); }

  public:
    Value() = default;
    WEBCFACE_DLL Value(const Field &base);
    Value(const Field &base, const std::string &field)
        : Value(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 子フィールドを返す
    /*!
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするValue
     */
    Value child(const std::string &field) {
        return Value{*this, this->field_ + "." + field};
    }

    //! Dictの値を再帰的にセットする
    Value &set(const Dict &v) {
        if (v.hasValue()) {
            set(v.getRaw());
        } else {
            for (const auto &it : v.getChildren()) {
                child(it.first).set(it.second);
            }
        }
        return *this;
    }
    //! 数値または配列をセットする
    Value &set(const VectorOpt<double> &v) {
        set(std::make_shared<VectorOpt<double>>(v));
        return *this;
    }

    //! Dictの値を再帰的にセットする
    Value &operator=(const Dict &v) {
        this->set(v);
        return *this;
    }
    //! 数値または配列をセットする
    Value &operator=(const VectorOpt<double> &v) {
        this->set(v);
        return *this;
    }

    //! 値を返す
    std::optional<double> tryGet() const {
        auto v = getRaw();
        if (v) {
            return **v;
        } else {
            return std::nullopt;
        }
    }
    //! 値をvectorで返す
    std::optional<std::vector<double>> tryGetVec() const {
        auto v = getRaw();
        if (v) {
            return **v;
        } else {
            return std::nullopt;
        }
    }
    //! 値を再帰的に取得しDictで返す
    std::optional<Dict> tryGetRecurse() const {
        return getRawRecurse();
    }
    //! 値を返す
    double get() const { return tryGet().value_or(0); }
    //! 値をvectorで返す
    std::vector<double> getVec() const {
        return tryGetVec().value_or(std::vector<double>{});
    }
    //! 値を再帰的に取得しDictで返す
    Dict getRecurse() const { return tryGetRecurse().value_or(Dict{}); }
    operator double() const { return get(); }
    operator std::vector<double>() const { return getVec(); }
    operator Dict() const { return getRecurse(); }
    //! syncの時刻を返す
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL Value &free();

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
    //! 1足したものをsetした後自身を返す
    Value &operator++() { // ++s
        this->set(this->get() + 1);
        return *this;
    }
    //! 1足したものをsetし、足す前の値を返す
    double operator++(int) { // s++
        auto v = this->get();
        this->set(v + 1);
        return v;
    }
    //! 1引いたものをsetした後自身を返す
    Value &operator--() { // --s
        this->set(this->get() - 1);
        return *this;
    }
    //! 1引いたものをsetし、足す前の値を返す
    double operator--(int) { // s--
        auto v = this->get();
        this->set(v - 1);
        return v;
    }

    // 比較演算子の定義は不要
};


inline std::ostream &operator<<(std::ostream &os, const Value &data) {
    return os << data.get();
}

} // namespace WebCFace
