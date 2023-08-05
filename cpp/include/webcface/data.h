#pragma once
#include <string>
#include <istream>
#include <ostream>
#include <optional>
#include "data_store.h"
#include "event.h"
#include "decl.h"

namespace WebCFace {

//! データの参照先を表すクラス。
/*! SyncDataとSyncDataStoreの関数定義はヘッダーに書いてないが、
 * ソース(client/data.cc)に書いて、特定の型についてインスタンス化する
 */
template <typename T>
class SyncData {
  protected:
    Client *cli;
    //! cliが持っているstoreを表すがTによって参照先が違う
    SyncDataStore<T> *store;

    std::string member_, name_;

  public:
    using DataType = T;
    SyncData() {}
    SyncData(Client *cli, const std::string &member, const std::string &name);
    //! 参照先のMemberを返す
    Member member() const;
    //! 参照先のデータ名を返す
    std::string name() const { return name_; }

    //! 値をセットする
    void set(const T &data);
    //! 値を取得する
    std::optional<T> try_get() const;
    //! 値を取得する
    //! todo: 引数
    T get() const;
    //! 値を取得する
    operator T() const { return this->get(); }
};

template <typename T>
auto &operator>>(std::basic_istream<char> &is, SyncData<T> &data) {
    T v;
    is >> v;
    data.set(v);
    return is;
}
template <typename T>
auto &operator>>(std::basic_istream<char> &is, SyncData<T> &&data) {
    SyncData<T> d = data;
    return is >> d;
}
template <typename T>
auto &operator<<(std::basic_ostream<char> &os, const SyncData<T> &data) {
    return os << data.get();
}


//! 実数値を扱う
class Value : public SyncData<double>, public EventTarget<Value> {
  public:
    Value() = default;
    Value(Client *cli, const std::string &member, const std::string &name);
    Value(const EventKey &key) : Value(key.cli, key.member, key.name) {}

    //! 値をセットし、EventTargetを発動するためoverload
    auto &set(double data) {
        this->SyncData<double>::set(data);
        this->triggerEvent();
        return *this;
    }
    //! 値をセットする
    auto &operator=(double data) {
        this->set(data);
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
class Text : public SyncData<std::string>, public EventTarget<Text> {
  public:
    Text() = default;
    Text(Client *cli, const std::string &member, const std::string &name);
    Text(const EventKey &key) : Text(key.cli, key.member, key.name) {}

    //! 値をセットし、EventTargetを発動するためoverload
    auto &set(const std::string &data) {
        this->SyncData<std::string>::set(data);
        this->triggerEvent();
        return *this;
    }
    //! 値をセットする
    auto &operator=(const std::string &data) {
        this->set(data);
        return *this;
    }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};


} // namespace WebCFace
