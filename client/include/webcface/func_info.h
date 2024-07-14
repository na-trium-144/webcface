#pragma once
#include <future>
#include <vector>
#include <type_traits>
#include <functional>
#include <optional>
#include <string>
#include <ostream>
#include <cstddef>
#include "webcface/encoding/val_adaptor.h"
#include "webcface/common/def.h"

#ifdef min
// clang-format off
#pragma message("warning: Disabling macro definition of 'min' and 'max', since they conflicts in webcface/func_info.h.")
// clang-format on
#undef min
#undef max
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct Arg;
struct Call;
struct FuncInfo;
} // namespace message
namespace internal {
struct ClientData;
class AsyncFuncState;
} // namespace internal

/*!
 * \brief 引数の情報を表す。
 *
 * func.setArg({ Arg(引数名).init(初期値).min(最小値).max(最大値), ... });
 * のように使う
 *
 */
class WEBCFACE_DLL Arg {
  protected:
    SharedString name_;
    ValType type_ = ValType::none_;
    std::optional<ValAdaptor> init_ = std::nullopt;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt;
    std::vector<ValAdaptor> option_;

  public:
    /*!
     * \brief 引数のargが持っている情報でthisを上書きする。
     *
     */
    void mergeConfig(const Arg &rhs);
    explicit Arg(ValType type) : type_(type) {}
    Arg() = default;

    message::Arg toMessage() const;
    Arg(const message::Arg &a);

    /*!
     * \brief 引数名を設定する。
     *
     */
    Arg(std::string_view name) : name_(SharedString::encode(name)) {}
    Arg(std::wstring_view name) : name_(SharedString::encode(name)) {}

    /*!
     * \brief 引数の名前を取得する。
     *
     */
    std::string name() const { return name_.decode(); }
    /*!
     * \brief 引数の名前を取得する。(wstring)
     * \since ver2.0
     */
    std::wstring nameW() const { return name_.decodeW(); }
    /*!
     * \brief 引数の型を取得する。
     *
     */
    ValType type() const { return type_; }
    /*!
     * \brief 引数の型を設定する。
     *
     */
    Arg &type(ValType type) {
        type_ = type;
        return *this;
    }
    /*!
     * \brief デフォルト値を取得する。
     *
     */
    std::optional<ValAdaptor> init() const { return init_; }
    /*!
     * \brief デフォルト値を設定する。
     *
     */
    template <typename T>
    Arg &init(const T &init) {
        init_ = ValAdaptor(init);
        return *this;
    }
    /*!
     * \brief 最小値を取得する。
     *
     */
    std::optional<double> min() const { return min_; }
    /*!
     * \brief 最小値を設定する。
     *
     * * string型引数の場合最小の文字数を表す。
     * * bool型引数の場合効果がない。
     * * option() はクリアされる。
     *
     */
    Arg &min(double min) {
        min_ = min;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 最大値を取得する。
     *
     */
    std::optional<double> max() const { return max_; }
    /*!
     * \brief 最大値を設定する。
     *
     * * string型引数の場合最大の文字数を表す。
     * * bool型引数の場合効果がない。
     * * option() はクリアされる。
     *
     */
    Arg &max(double max) {
        max_ = max;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 引数の選択肢を取得する。
     *
     */
    const std::vector<ValAdaptor> &option() const { return option_; }
    Arg &option(std::vector<ValAdaptor> option) {
        option_ = std::move(option);
        min_ = max_ = std::nullopt;
        return *this;
    }
    /*!
     * \brief 引数の選択肢を設定する。
     *
     * * min(), max() はクリアされる。
     *
     */
    template <typename T>
    Arg &option(std::initializer_list<T> option) {
        std::vector<ValAdaptor> option_v;
        for (const auto &v : option) {
            option_v.emplace_back(ValAdaptor(v));
        }
        return this->option(std::move(option_v));
    }
};
WEBCFACE_DLL std::ostream &operator<<(std::ostream &os, const Arg &arg);

/*!
 * \brief 関数1つの情報を表す。関数の実体も持つ
 *
 * 元の関数が値を返す場合、futureは即座に返るので非同期にする意味がない
 * → eval_async=false にする
 *
 */
struct FuncInfo {
    using FuncType = std::future<ValAdaptor>(std::vector<ValAdaptor>);

    ValType return_type;
    std::vector<Arg> args;
    bool eval_async;
    std::function<FuncType> func_impl;

    /*!
     * \brief func_implをこのスレッドで実行
     *
     * * eval_async=caller_async=trueならfutureを新しいスレッドで評価し、
     * そうでなければこのスレッドでwaitする
     * * 建てたスレッドはdetachする
     * * stateがnullptrでなければstate->setResultFuture()にfutureをセットし、
     * 実行完了後にstate->callResultEvent()を呼ぶ
     * * 発生した例外はcatchしない
     */
    std::shared_future<ValAdaptor>
    run(const std::vector<ValAdaptor> &args, bool caller_async,
        const std::shared_ptr<internal::AsyncFuncState> &state = nullptr);
    /*!
     * \brief
     * func_implをこのスレッドで実行し、完了時にCallResultをdataに送信させる
     *
     * * eval_async=trueなら新しいスレッドで、そうでなければこのスレッドでfutureを評価
     * * 建てたスレッドはdetachする
     *
     */
    void run(webcface::message::Call &&call,
             const std::shared_ptr<internal::ClientData> &data);

    FuncInfo()
        : return_type(ValType::none_), args(), eval_async(false), func_impl() {}
    FuncInfo(ValType return_type, std::vector<Arg> args, bool async,
             std::function<FuncType> func_impl)
        : return_type(return_type), args(std::move(args)), eval_async(async),
          func_impl(std::move(func_impl)) {}
    FuncInfo(const message::FuncInfo &m);
    message::FuncInfo toMessage(const SharedString &field) const;
};

WEBCFACE_NS_END
