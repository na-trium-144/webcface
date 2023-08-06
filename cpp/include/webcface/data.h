#pragma once
#include <istream>
#include <ostream>
#include <optional>
#include <cassert>
#include "field_base.h"
#include "client_data.h"
#include "event_target.h"

namespace WebCFace {

//! フィールドを参照するインタフェースを持つクラス
template <typename T>
class SyncFieldBase : protected FieldBase {
  public:
    SyncFieldBase() = default;
    SyncFieldBase(const FieldBase &base) : FieldBase(base) {}

    //! Memberを返す
    Member member() const { return *this; }
    //! field名を返す
    std::string name() const { return field_; }


  protected:
    void setCheck() const {
        assert(dataLock()->isSelf(*this) &&
               "Cannot set data to member other than self");
    }

  public:
    //! 値を取得できるかチェックする
    //! 取得できなければリクエストする
    virtual std::optional<T> tryGet() const = 0;
    //! 値を取得する
    //! todo: 引数でタイムアウトとか設定できるようにする
    T get() const {
        auto v = tryGet();
        if (v) {
            return *v;
        } else {
            return T{};
        };
    }
    //! 値を取得する
    operator T() const { return get(); }
};

//! 実数値を扱う
class Value : public SyncFieldBase<double>, public EventTarget<Value> {
    using SyncFieldBase<double>::FieldBase::dataLock;

  public:
    Value() = default;
    Value(const FieldBase &base)
        : SyncFieldBase<double>(base),
          EventTarget<Value>(EventType::value_change, base,
                             [this] { this->tryGet(); }) {}
    Value(const FieldBase &base, const std::string &field)
        : Value(FieldBase{base, field}) {}

    //! 値をセットし、EventTargetを発動する
    auto &set(double v) {
        setCheck();
        dataLock()->value_store.setSend(*this, v);
        this->triggerEvent();
        return *this;
    }

    //! 値をセットする
    auto &operator=(double v) {
        this->set(v);
        return *this;
    }

    //! 値を取得する
    std::optional<double> tryGet() const override {
        return dataLock()->value_store.getRecv(*this);
    }

    auto &operator+=(double rhs) {
        this->set(this->get() + rhs);
        return *this;
    }
    auto &operator-=(double rhs) {
        this->set(this->get() - rhs);
        return *this;
    }
    auto &operator*=(double rhs) {
        this->set(this->get() * rhs);
        return *this;
    }
    auto &operator/=(double rhs) {
        this->set(this->get() / rhs);
        return *this;
    }
    auto &operator%=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) % rhs);
        return *this;
    }
    // int64_tも使えるようにすべきか?
    // javascriptで扱える整数は2^53まで
    auto &operator<<=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) << rhs);
        return *this;
    }
    auto &operator>>=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) >> rhs);
        return *this;
    }
    auto &operator&=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) & rhs);
        return *this;
    }
    auto &operator|=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) | rhs);
        return *this;
    }
    auto &operator^=(std::int32_t rhs) {
        this->set(static_cast<std::int32_t>(this->get()) ^ rhs);
        return *this;
    }
    auto &operator++() { // ++s
        this->set(this->get() + 1);
        return *this;
    }
    auto operator++(int) { // s++
        auto v = this->get();
        this->set(v + 1);
        return v;
    }
    auto &operator--() { // --s
        this->set(this->get() - 1);
        return *this;
    }
    auto operator--(int) { // s--
        auto v = this->get();
        this->set(v - 1);
        return v;
    }

    // 比較演算子の定義は不要
};

// 文字列を扱う
class Text : public SyncFieldBase<std::string>, public EventTarget<Text> {
    using SyncFieldBase<std::string>::FieldBase::dataLock;

  public:
    Text() = default;
    Text(const FieldBase &base)
        : SyncFieldBase<std::string>(base),
          EventTarget<Text>(EventType::text_change, base,
                            [this] { this->tryGet(); }) {}
    Text(const FieldBase &base, const std::string &field)
        : Text(FieldBase{base, field}) {}

    //! 値をセットし、EventTargetを発動する
    auto &set(const std::string &v) {
        setCheck();
        dataLock()->text_store.setSend(*this, v);
        triggerEvent();
        return *this;
    }

    //! 値をセットする
    auto &operator=(const std::string &v) {
        this->set(v);
        return *this;
    }

    //! 値を取得する
    std::optional<std::string> tryGet() const override {
        return dataLock()->text_store.getRecv(*this);
    }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};

inline auto &operator<<(std::basic_ostream<char> &os, const Value &data) {
    return os << data.get();
}
inline auto &operator<<(std::basic_ostream<char> &os, const Text &data) {
    return os << data.get();
}
} // namespace WebCFace
