#pragma once
#include <string>
#include <memory>
#include <optional>
#include <cstdint>
#include <functional>
#include <type_traits>
#include "data_store.h"
#include "func.h"
#include "any_arg.h"

namespace WebCFace {

class Client;

template <typename T>
class SyncData {
  private:
    std::shared_ptr<SyncDataStore<T>> store;

  protected:
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

class Func : public SyncData<FuncInfo> {
    std::shared_ptr<FuncStore> func_impl_store;
    template <typename... Args, typename Ret>
    void set_impl(std::function<Ret(Args...)> func) {
        this->SyncData<DataType>::set(FuncInfo{func});
        func_impl_store->set(name, [func](std::vector<AnyArg> args_vec) {
            std::tuple<Args...> args_tuple;
            argToTuple(args_vec, args_tuple);
            if constexpr (std::is_void_v<Ret>) {
                std::apply(func, args_tuple);
                return AnyArg{};
            } else {
                Ret ret = std::apply(func, args_tuple);
                return static_cast<AnyArg>(ret);
            }
        });
    }

  public:
    Func(std::shared_ptr<SyncDataStore<DataType>> store,
         std::shared_ptr<FuncStore> func_impl_store,
         const std::string &from, const std::string &name)
        : SyncData<DataType>(store, from, name), func_impl_store(func_impl_store) {}

    template <typename T>
    void set(const T &func) {
        this->set_impl(std::function(func));
    }
    template <typename T>
    auto &operator=(const T &func) {
        this->set(func);
        return *this;
    }

    template <typename... Args>
    AnyArg run(Args... args) {
        std::tuple<Args...> args_tuple = {args...};
        std::vector<AnyArg> args_vec(sizeof...(args));
        tupleToArg(args_vec, args_tuple);
        return func_impl_store->get(name).operator()(args_vec);
    }
};

} // namespace WebCFace
