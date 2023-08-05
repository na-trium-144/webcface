#pragma once
#include <string>
#include <istream>
#include <ostream>
#include <optional>

namespace WebCFace {

//! データの参照先を表すクラス。
template <typename T>
class SyncData {
  protected:
    std::shared_ptr<ClientData> data;
    std::string member_, name_;

  public:
    SyncData(const std::shared_ptr<ClientData> &data, const std::string &member,
             const std::string &name)
        : data(data), member_(member), name_(name) {}

    //! 参照先のMemberを返す
    Member member() const { return Member{data, member_}; }
    //! 参照先のデータ名を返す
    std::string name() const { return name_; }

    //! 値を取得する
    virtual std::optional<T> tryGet() const = 0;
    //! 値を取得する
    //! todo: 引数
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

/*template <typename T>
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
}*/
template <typename T>
auto &operator<<(std::basic_ostream<char> &os, const SyncData<T> &data) {
    return os << data.get();
}

//! 実数値を扱う
class Value : public SyncData<double>, public EventTarget<Value> {
  public:
    Value() = default;
    Value(const std::shared_ptr<ClientData> &data, const std::string &member,
          const std::string &name)
        : SyncData<double>(data, member, name),
          EventTarget<Value>(data, EventType::value_change, member, name, [this] { this->tryGet(); }){}
    Value(const EventKey &key, const std::shared_ptr<ClientData> &data)
        : Value(data, key.member, key.name) {}

    //! 値をセットし、EventTargetを発動するためoverload
    auto &set(double v) {
        assert(member_ == "" && "Cannot set data to member other than self");
        this->SyncData<double>::data->value_store.setSend(name_, v);
        this->triggerEvent();
        return *this;
    }
    //! 値を取得する
    std::optional<double> tryGet() const override {
        return this->SyncData<double>::data->value_store.getRecv(member_, name_);
    }

    //! 値をセットする
    auto &operator=(double v) {
        this->set(v);
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
    Text(const std::shared_ptr<ClientData> &data, const std::string &member,
         const std::string &name)
        : SyncData<std::string>(data, member, name),
          EventTarget<Text>(data, EventType::text_change, member, name, [this] { this->tryGet(); }){}
    Text(const EventKey &key, const std::shared_ptr<ClientData> &data)
        : Text(data, key.member, key.name) {}

    //! 値をセットし、EventTargetを発動するためoverload
    auto &set(const std::string &v) {
        assert(member_ == "" && "Cannot set data to member other than self");
        this->SyncData<std::string>::data->text_store.setSend(name_, v);
        this->triggerEvent();
        return *this;
    }
    //! 値を取得する
    std::optional<std::string> tryGet() const override {
        return this->SyncData<std::string>::data->text_store.getRecv(member_, name_);
    }
    //! 値をセットする
    auto &operator=(const std::string &v) {
        this->set(v);
        return *this;
    }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};


} // namespace WebCFace
