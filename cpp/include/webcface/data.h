#pragma once
#include <string>
#include <memory>
#include <optional>
#include <cstdint>

namespace WebCFace {

class Client;

template <typename T>
class SyncData {
  private:
    std::shared_ptr<SyncDataStore<T>> store;
    std::string from, name;

  public:
    using DataType = T;
    SyncData(std::shared_ptr<SyncDataStore<T>> store, const std::string &from,
             const std::string &name)
        : store(store), from(from), name(name) {}
    void set(const T &data);
    std::optional<T> try_get() const;
    T get() const;
    operator T() const { return this->get(); }
};

class Value : public SyncData<double> {
  public:
    Value(std::shared_ptr<SyncDataStore<DataType>> store,
          const std::string &from, const std::string &name)
        : SyncData<DataType>(store, from, name) {}
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
};

class Text : public SyncData<std::string> {
  public:
    Text(std::shared_ptr<SyncDataStore<DataType>> store,
         const std::string &from, const std::string &name)
        : SyncData<DataType>(store, from, name) {}
    auto &operator=(const std::string &data) {
        this->set(data);
        return *this;
    }
};

} // namespace WebCFace
