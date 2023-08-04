#pragma once
#include <string>
#include <cstdint>
#include <eventpp/eventdispatcher.h>
#include "data.h"
#include "decl.h"

namespace WebCFace {

//! SyncDataをMapなどのキーにするために、memberとnameを比較するためのクラス
template <typename T>
struct SyncDataKey {
    std::string member, name;
    SyncDataKey(const std::string &member, const std::string &name)
        : member(member), name(name) {}
    SyncDataKey(const SyncData<T> &data)
        : member(data.member().name()), name(data.name()) {}
    bool operator==(const SyncDataKey &rhs) const {
        return member == rhs.member && name == rhs.name;
    }
    bool operator!=(const SyncDataKey &rhs) const { return !(*this == rhs); }
    bool operator<(const SyncDataKey &rhs) const {
        return member < rhs.member || (member == rhs.member && name < rhs.name);
    }
};
template <typename T, typename V>
class SyncDataWithEvent : public SyncData<T> {
  public:
    using EventDispatcher =
        eventpp::EventDispatcher<SyncDataKey<T>, void(const V &)>;
    using EventHandle = EventDispatcher::Handle;
    using EventCallback = EventDispatcher::Callback;

  private:
    EventDispatcher *dispatcher = nullptr;

  public:
    SyncDataWithEvent() = default;
    SyncDataWithEvent(Client *cli, const std::string &member,
                      const std::string &name);

    //! 値をセットし、イベントを発生させる
    virtual void set(const T &data) override;

    EventHandle appendListener(const EventCallback &callback) const {
        this->try_get();
        return dispatcher->appendListener(*this, callback);
    }
    EventHandle prependListener(const EventCallback &callback) const {
        this->try_get();
        return dispatcher->prependListener(*this, callback);
    }
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle before) const {
        this->try_get();
        return dispatcher->insertListener(*this, callback, before);
    }
    bool removeListener(const EventHandle handle) const {
        return dispatcher->removeListener(*this, handle);
    }
    bool hasAnyListener() const { return dispatcher->hasAnyListener(*this); }
    bool ownsHandle(const EventHandle &handle) const {
        return dispatcher->ownsHandle(*this, handle);
    }
};

//! 実数値を扱う
class Value : public SyncDataWithEvent<double, Value> {
  public:
    Value() = default;
    Value(Client *cli, const std::string &member, const std::string &name)
        : SyncDataWithEvent<double, Value>(cli, member, name) {}
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
class Text : public SyncDataWithEvent<std::string, Text> {
  public:
    Text() {}
    Text(Client *cli, const std::string &member, const std::string &name)
        : SyncDataWithEvent<std::string, Text>(cli, member, name) {}
    //! 値をセットする
    auto &operator=(const std::string &data) {
        this->set(data);
        return *this;
    }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};


} // namespace WebCFace
