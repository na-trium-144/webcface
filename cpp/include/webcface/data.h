#pragma once
#include <istream>
#include <ostream>
#include <optional>
#include <chrono>
#include "field.h"
#include "client_data.h"
#include "event_target.h"

namespace WebCFace {

//! 実数値を扱う
class Value
    : protected Field,
      public EventTarget<Value, decltype(ClientData::value_change_event)> {
    std::optional<double> value_;
    std::optional<std::chrono::system_clock::time_point> time_;

    void onAppend() const override { setCheck(); }

  public:
    Value() = default;
    Value(const Field &base)
        : Field(base),
          EventTarget<Value, decltype(ClientData::value_change_event)>(
              &this->dataLock()->value_change_event, *this) {
        auto data = dataLock();
        value_ = data->value_store.getRecv(*this);
        time_ = data->sync_time_store.getRecv(this->member_);
    }
    Value(const Field &base, const std::string &field)
        : Value(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 値をセットし、EventTargetを発動する
    auto &set(double v) {
        setCheck();
        dataLock()->value_store.setSend(*this, v);
        value_ = v;
        this->triggerEvent(*this);
        return *this;
    }
    //! 値をセットし、EventTargetを発動する
    auto &operator=(double v) {
        this->set(v);
        return *this;
    }

    //! このvalueを非表示にする
    //! (他clientのentryに表示されなくする)
    auto &hidden(bool hidden) {
        setCheck();
        dataLock()->value_store.setHidden(*this, hidden);
        return *this;
    }

    //! 値を返す
    std::optional<double> tryGet() const { return value_; }
    double get() const { return tryGet().value_or(0); }
    operator double() const { return get(); }

    //! 値やリクエスト状態をクリア
    auto &free() {
        dataLock()->value_store.unsetRecv(*this);
        value_ = std::nullopt;
        return *this;
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
class Text : protected Field,
             public EventTarget<Text, decltype(ClientData::text_change_event)> {
    std::optional<std::string> value_;
    std::optional<std::chrono::system_clock::time_point> time_;

    void onAppend() const override { setCheck(); }

  public:
    Text() = default;
    Text(const Field &base)
        : Field(base),
          EventTarget<Text, decltype(ClientData::text_change_event)>(
              &this->dataLock()->text_change_event, *this) {
        auto data = dataLock();
        value_ = data->text_store.getRecv(*this);
        time_ = data->sync_time_store.getRecv(this->member_);
    }
    Text(const Field &base, const std::string &field)
        : Text(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 値をセットし、EventTargetを発動する
    auto &set(const std::string &v) {
        setCheck();
        dataLock()->text_store.setSend(*this, v);
        value_ = v;
        triggerEvent(*this);
        return *this;
    }
    //! 値をセットする
    auto &operator=(const std::string &v) {
        this->set(v);
        return *this;
    }

    //! 値を取得する
    std::optional<std::string> tryGet() const { return value_; }
    std::string get() const { return tryGet().value_or(""); }
    operator std::string() const { return get(); }

    //! このtext非表示にする
    //! (他clientのentryに表示されなくする)
    auto &hidden(bool hidden) {
        setCheck();
        dataLock()->text_store.setHidden(*this, hidden);
        return *this;
    }
    //! 関数の設定を解除
    auto &free() {
        dataLock()->text_store.unsetRecv(*this);
        return *this;
    }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};

//! log
//! name使わないんだけど
//! todo: triggerEvent
class Log
    : protected Field,
      public EventTarget<LogLine, decltype(ClientData::log_append_event)> {
    std::optional<std::vector<LogLine>> value_;

    void onAppend() const override { setCheck(); }

  public:
    Log() = default;
    Log(const Field &base)
        : Field(base),
          EventTarget<LogLine, decltype(ClientData::log_append_event)>(
              &this->dataLock()->log_append_event, this->member_) {
        value_ = this->dataLock()->log_store.getRecv(member_);
    }

    using Field::member;

    //! 値を取得する
    std::optional<std::vector<LogLine>> tryGet() const { return value_; }
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
