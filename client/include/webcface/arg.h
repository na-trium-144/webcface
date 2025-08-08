#pragma once
#include <vector>
#include <optional>
#include "webcface/common/val_adaptor.h"

#ifdef min
// clang-format off
#pragma message("warning: Disabling macro definition of 'min' and 'max', since they conflicts in webcface/arg.h.")
// clang-format on
#undef min
#undef max
#endif

WEBCFACE_NS_BEGIN

namespace message {
struct Arg;
}
namespace internal {
struct FuncInfo;
}

/*!
 * \brief 引数の情報を表す。
 *
 * func.setArg({ Arg(引数名).init(初期値).min(最小値).max(最大値), ... });
 * のように使う
 *
 */
class WEBCFACE_DLL Arg {
    std::shared_ptr<message::Arg> msg_data;

    /*!
     * Funcの初期状態などtype以外不要な場合があるので、typeだけpimplとは別にしている
     *
     */
    ValType type_ = ValType::none_;

    /*!
     * msg_dataが空なら初期化して返す
     *
     */
    std::shared_ptr<message::Arg> initMsg();

  public:
    friend struct internal::FuncInfo;
    /*!
     * \brief 引数のargが持っている情報でthisを上書きする。
     *
     */
    void mergeConfig(const Arg &other);
    /*!
     * msg_dataはnullになる
     *
     */
    explicit Arg(ValType type = ValType::none_);
    /*!
     * msg_dataとtypeを初期化
     *
     */
    explicit Arg(const std::shared_ptr<message::Arg> &msg_data);

    /*!
     * \since ver2.0
     *
     */
    explicit Arg(const SharedString &name);
    /*!
     * \brief 引数名を設定する。
     *
     * ver2.0〜wstring対応、ver2.10〜 StringInitializer 型に変更
     */
    Arg(StringInitializer name) : Arg(static_cast<SharedString &>(name)) {}

    /*!
     * \brief 引数の名前を取得する。
     *
     * ver2.10〜 StringView に変更
     *
     */
    StringView name() const;
    /*!
     * \brief 引数の名前を取得する。(wstring)
     * \since ver2.0
     *
     * ver2.10〜 WStringView に変更
     *
     */
    WStringView nameW() const;
    /*!
     * \brief 引数の型を取得する。
     *
     */
    ValType type() const;
    /*!
     * \brief 引数の型を設定する。
     *
     */
    Arg &type(ValType type);
    /*!
     * \brief デフォルト値を取得する。
     *
     */
    std::optional<ValAdaptor> init() const;
    /*!
     * \brief デフォルト値を設定する。
     *
     */
    template <typename T>
    Arg &init(const T &init) {
        return this->init(ValAdaptor(init));
    }
    /*!
     * \since ver2.10
     */
    template <std::size_t N>
    Arg &init(const char (&init)[N]) {
        return this->init(ValAdaptor(init));
    }
    /*!
     * \since ver2.10
     */
    template <std::size_t N>
    Arg &init(const wchar_t (&init)[N]) {
        return this->init(ValAdaptor(init));
    }
    Arg &init(const ValAdaptor &init);
    /*!
     * \brief 最小値を取得する。
     *
     */
    std::optional<double> min() const;
    /*!
     * \brief 最小値を設定する。
     *
     * * string型引数の場合最小の文字数を表す。
     * * bool型引数の場合効果がない。
     * * option() はクリアされる。
     *
     */
    Arg &min(double min);
    /*!
     * \brief 最大値を取得する。
     *
     */
    std::optional<double> max() const;
    /*!
     * \brief 最大値を設定する。
     *
     * * string型引数の場合最大の文字数を表す。
     * * bool型引数の場合効果がない。
     * * option() はクリアされる。
     *
     */
    Arg &max(double max);
    /*!
     * \brief 引数の選択肢を取得する。
     *
     */
    const std::vector<ValAdaptor> &option() const;
    Arg &option(std::vector<ValAdaptor> option);
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
WEBCFACE_DLL std::ostream &WEBCFACE_CALL operator<<(std::ostream &os,
                                                    const Arg &arg);
WEBCFACE_NS_END
