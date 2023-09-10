#pragma once
#include <istream>
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/dict.h"
#include "field.h"
#include "client_data.h"
#include "event_target.h"

namespace WebCFace {

//! 実数値を扱う
class Value : protected Field, public EventTarget<Value> {

    void onAppend() const override { tryGet(); }

  public:
    Value() = default;
    Value(const Field &base)
        : Field(base), EventTarget<Value>(&this->dataLock()->value_change_event,
                                          *this) {}
    Value(const Field &base, const std::string &field)
        : Value(Field{base, field}) {}

    using Field::member;
    using Field::name;

    auto child(const std::string &field) {
        return Value{*this, this->field_ + "." + field};
    }

    using Dict = Common::Dict<std::shared_ptr<Common::VectorOpt<double>>>;
    Value &set(const Dict &v) {
        if (v.hasValue()) {
            set(v.get());
        } else {
            for (const auto &it : v.getChildren()) {
                child(it.first).set(it.second);
            }
        }
        return *this;
    }
    //! 値をセットし、EventTargetを発動する
    Value &set(const VectorOpt<double> &v) {
        setCheck();
        dataLock()->value_store.setSend(*this, v);
        this->triggerEvent(*this);
        return *this;
    }

    auto &operator=(const Dict &v) {
        this->set(v);
        return *this;
    }
    auto &operator=(const VectorOpt<double> &v) {
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
    std::optional<double> tryGet() const {
        return dataLock()->value_store.getRecv(*this);
    }
    std::optional<std::vector<double>> tryGetVec() const {
        return dataLock()->value_store.getRecv(*this);
    }
    std::optional<Dict> tryGetRecurse() const {
        return dataLock()->value_store.getRecvRecurse(*this);
    }
    double get() const { return tryGet().value_or(0); }
    std::vector<double> getVec() const { return tryGetVec().value_or(std::vector<double>{}); }
    Dict getRecurse() const { return tryGetRecurse().value_or(Dict{}); }
    operator double() const { return get(); }
    operator std::vector<double>() const { return getVec(); }
    operator Dict() const { return getRecurse(); }
    auto time() const {
        return dataLock()
            ->sync_time_store.getRecv(this->member_)
            .value_or(std::chrono::system_clock::time_point());
    }

    //! 値やリクエスト状態をクリア
    auto &free() {
        dataLock()->value_store.unsetRecv(*this);
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
class Text : protected Field, public EventTarget<Text> {

    void onAppend() const override { tryGet(); }

  public:
    Text() = default;
    Text(const Field &base)
        : Field(base), EventTarget<Text>(&this->dataLock()->text_change_event,
                                         *this) {}
    Text(const Field &base, const std::string &field)
        : Text(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 値をセットし、EventTargetを発動する
    auto &set(const std::string &v) {
        setCheck();
        dataLock()->text_store.setSend(*this, v);
        triggerEvent(*this);
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
    auto time() const {
        return dataLock()
            ->sync_time_store.getRecv(this->member_)
            .value_or(std::chrono::system_clock::time_point());
    }

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
class Log : protected Field, public EventTarget<Log, std::string> {

    void onAppend() const override { tryGet(); }

  public:
    Log() = default;
    Log(const Field &base)
        : Field(base), EventTarget<Log, std::string>(
                           &this->dataLock()->log_append_event, this->member_) {
    }

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
