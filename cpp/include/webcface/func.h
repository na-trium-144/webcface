#pragma once
#include <vector>
#include <type_traits>
#include <functional>
#include "any_arg.h"
namespace WebCFace {
template <typename T>
static AbstArgType abstTypeOf() {
    if constexpr (std::is_void<T>) {
        return AbstArgType::none_;
    } else if constexpr (std::is_same_v<bool, T>) {
        return AbstArgType::bool_;
    } else if constexpr (std::is_integral_v<T>) {
        return AbstArgType::int_;
    } else if constexpr (std::is_floating_point_v<T>) {
        return AbstArgType::float_;
    } else {
        return AbstArgType::string_;
    }
}
struct FuncInfo {
    AbstArgType return_type;
    std::vector<AbstArgType> args_type;
    template <typename... Args, typename Ret>
    explicit FuncInfo(std::function<Ret(Args...)>)
        : return_type(abstTypeOf<Ret>()), args_type({abstTypeOf<Args>()...}) {}
};

class FuncStore {
  public:
    using FuncType = std::function<AnyArg(std::vector<AnyArg>)>;

  private:
    std::mutex mtx;
    std::unordered_map<std::string, FuncType> funcs;

  public:
    void set(const std::string &name, FuncType data);
    FuncType get(const std::string &from, const std::string &name);
    std::unordered_map<std::string, T> transfer_send();
    std::set<std::pair<std::string, std::string>> transfer_subsc();
};

} // namespace WebCFace