#pragma once
#include <istream>
#include <ostream>
#include <optional>
#include "field.h"
#include "client_data.h"
#include "event_target.h"

namespace WebCFace {

//! 実数値を扱う
class Value : protected Field, public EventTarget<Value> {
  public:
    Value() = default;
    Value(const Field &base)
        : Field(base), EventTarget<Value>(EventType::value_change, base,
                                          [this] { this->tryGet(); }) {}
    Value(const Field &base, const std::string &field)
        : Value(Field{base, field}) {}

    using Field::member;
    using Field::name;

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
    std::optional<double> tryGet() const {
        return dataLock()->value_store.getRecv(*this);
    }
    double get() const { return tryGet().value_or(0); }
    operator double() const { return get(); }

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
class Text : protected Field, public EventTarget<Text> {
  public:
    Text() = default;
    Text(const Field &base)
        : Field(base), EventTarget<Text>(EventType::text_change, base,
                                         [this] { this->tryGet(); }) {}
    Text(const Field &base, const std::string &field)
        : Text(Field{base, field}) {}

    using Field::member;
    using Field::name;

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
    std::optional<std::string> tryGet() const {
        return dataLock()->text_store.getRecv(*this);
    }
    std::string get() const { return tryGet().value_or(""); }
    operator std::string() const { return get(); }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};

//! log
//! name使わないんだけど
//! todo: triggerEvent
class Log : protected Field, public EventTarget<Log> {
  public:
    Log() = default;
    Log(const Field &base)
        : Field(base), EventTarget<Log>(EventType::log_change, base,
                                        [this] { this->tryGet(); }) {}

    using Field::member;

    //! 値を取得する
    std::optional<std::vector<LogLine>> tryGet() const {
        return dataLock()->log_store.getRecv(member_);
    }
    std::vector<LogLine> get() const {
        return tryGet().value_or(std::vector<LogLine>{});
    }
};

inline auto &operator<<(std::ostream &os, const Value &data) {
    return os << data.get();
}
inline auto &operator<<(std::ostream &os, const Text &data) {
    return os << data.get();
}
} // namespace WebCFace
