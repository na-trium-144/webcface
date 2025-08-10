#pragma once
#include "webcface/common/val_adaptor_vec.h"
#include "func_result.h"
#include "arg.h"
#include "webcface/common/trait.h"
#include "func_trait.h"
#include "exception.h"

WEBCFACE_NS_BEGIN
namespace internal {
struct FuncInfo;
}

namespace traits {
template <typename BadArg>
struct This_arg_type_is_not_supported_by_WebCFace_Func {};
template <typename BadArg>
struct FuncArgTypesIterationFailureTrait {
    using ArgTypesCheckResult =
        This_arg_type_is_not_supported_by_WebCFace_Func<BadArg>;
};
// MSVCでexplicitな変換operatorがあるとき
// std::is_constructible_t<Arg, ValAdaptor> が正しくtrueを返さないため、
// decltypeで実装している
template <typename Arg, typename = void>
struct IsConstructibleArg : std::false_type {};
template <>
struct IsConstructibleArg<ValAdaptor, void> : std::true_type {};
template <>
struct IsConstructibleArg<ValAdaptorVector, void> : std::true_type {};
template <typename Arg>
struct IsConstructibleArg<
    Arg, std::void_t<decltype(std::declval<ValAdaptorVector>().operator Arg())>>
    : std::true_type {};

template <typename... Args>
struct FuncArgTypesIterationTrait;
template <>
struct FuncArgTypesIterationTrait<> {
    struct ArgTypesCheckResult {
        using ArgTypesCheckOk = TraitOkType;
    };
};
template <typename FirstArg, typename... OtherArgs>
struct FuncArgTypesIterationTrait<FirstArg, OtherArgs...> {
    using ArgTypesCheckResult = typename std::conditional_t<
        IsConstructibleArg<std::decay_t<FirstArg>>::value,
        FuncArgTypesIterationTrait<OtherArgs...>,
        FuncArgTypesIterationFailureTrait<FirstArg>>::ArgTypesCheckResult;
};
template <typename... Args>
using FuncArgTypesTrait =
    typename FuncArgTypesIterationTrait<Args...>::ArgTypesCheckResult;

template <typename BadArg>
struct This_return_type_is_not_supported_by_WebCFace_Func {};
struct FuncReturnTypeCheckOkTrait {
    using FuncReturnTypeCheckOk = TraitOkType;
};
template <typename Ret>
using FuncReturnTypeTrait = std::conditional_t<
    std::disjunction_v<std::is_void<Ret>,
                       std::is_constructible<ValAdaptor, Ret>>,
    FuncReturnTypeCheckOkTrait,
    This_return_type_is_not_supported_by_WebCFace_Func<Ret>>;

/*!
 * RetとArgsが条件を満たすときだけ、
 * ReturnTypeTrait::FuncReturnTypeCheckOk と
 * ArgTypesTrait::FuncArgTypesCheckOk が定義される
 * (enable_ifを使ってないのはエラーメッセージがわかりにくかったから)
 *
 */
template <typename T>
struct FuncSignatureTrait {};
template <typename Ret, typename... Args>
struct FuncSignatureTrait<Ret(Args...)> : InvokeSignatureTrait<Ret(Args...)> {
    using ReturnTypeTrait = FuncReturnTypeTrait<Ret>;
    using ArgTypesTrait = FuncArgTypesTrait<Args...>;
    static constexpr bool return_void = std::is_void_v<Ret>;
    static inline bool assertArgsNum(const CallHandle &handle) {
        return handle.assertArgsNum(sizeof...(Args));
    }
    static inline std::vector<Arg> argsInfo() {
        return std::vector<Arg>{Arg{valTypeOf<Args>()}...};
    }
};
template <typename Ret, typename T, typename... Args>
struct FuncSignatureTrait<Ret (T::*)(Args...)>
    : FuncSignatureTrait<Ret(Args...)> {};
template <typename Ret, typename T, typename... Args>
struct FuncSignatureTrait<Ret (T::*)(Args...) const>
    : FuncSignatureTrait<Ret(Args...)> {};
template <typename Ret, typename... Args>
struct FuncSignatureTrait<Ret (*)(Args...)> : FuncSignatureTrait<Ret(Args...)> {
};

template <typename T>
using FuncObjTrait = FuncSignatureTrait<decltype(getInvokeSignature(
    std::declval<std::decay_t<T>>()))>;

} // namespace traits

/*!
 * \brief 関数1つを表すクラス
 *
 */
class WEBCFACE_DLL Func : protected Field {
  public:
    friend class TemporalViewComponent;
    friend class TemporalCanvas2DComponent;

    Func() = default;
    Func(const Field &base);
    Func(const Field &base, const SharedString &field)
        : Func(Field{base, field}) {}

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     *
     */
    Func child(StringInitializer field) const {
        return this->Field::child(static_cast<SharedString &>(field));
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Func child(int index) const {
        return this->Field::child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     *
     * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     *
     */
    Func operator[](StringInitializer field) const {
        return child(std::move(field));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Func operator[](int index) const {
        return child(std::to_string(index));
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Func parent() const { return this->Field::parent(); }

    using FuncType = void WEBCFACE_CALL_FP(CallHandle);

  protected:
    /*!
     * set2()で構築された関数の情報(FuncInfo)をclientにセット
     *
     */
    const Func &setImpl(ValType return_type, std::vector<Arg> &&args,
                        std::function<FuncType> &&func_impl) const;
    /*!
     * 引数の個数不定バージョン
     *
     */
    const Func &setImpl(ValType return_type, std::nullopt_t,
                        std::function<FuncType> &&func_impl) const;
    const Func &
    setImpl(const std::shared_ptr<internal::FuncInfo> &func_info) const;

    /*!
     * f_run()を実行し例外を投げた場合はrejectする
     *
     */
    template <typename F1>
    static void catchAll(F1 &&f_run, const CallHandle &handle) {
        ValAdaptor error;
        try {
            f_run();
            return;
        } catch (const std::exception &e) {
            error = e.what();
        } catch (const std::string &e) {
            error = e;
        } catch (const char *e) {
            error = e;
        } catch (const std::wstring &e) {
            error = e;
        } catch (const wchar_t *e) {
            error = e;
        } catch (...) {
            error = "unknown exception";
        }
        handle.reject(error);
    }
    /*!
     * f_run()を実行し結果をCallHandleに渡す
     *
     */
    template <typename F1>
    static void tryRun(F1 &&f_run, const CallHandle &handle) {
        catchAll([&] { handle.respond(f_run()); }, handle);
    }

  public:
    /*!
     * \brief 関数をセットする
     *
     * * ver2.0〜: set()でセットした場合、他クライアントから呼び出されたとき
     * Client::recv() (または autoRecv) のスレッドでそのまま実行され、
     * この関数が完了するまで他のデータの受信はブロックされる。
     * また、 runAsync() で呼び出したとしても同じスレッドで同期実行される。
     *
     * \param func セットする関数または関数オブジェクト。
     * 引数はValAdaptorからキャスト可能な型ならいくつでも、
     * 戻り値はvoidまたはValAdaptorにキャスト可能な型が使用可能
     * \sa setAsync()
     */
    template <typename T,
              typename traits::FuncObjTrait<
                  T>::ReturnTypeTrait::FuncReturnTypeCheckOk = traits::TraitOk,
              typename traits::FuncObjTrait<T>::ArgTypesTrait::ArgTypesCheckOk =
                  traits::TraitOk>
    const Func &set(T func) const {
        return setImpl(
            valTypeOf<typename traits::FuncObjTrait<T>::ReturnType>(),
            traits::FuncObjTrait<T>::argsInfo(),
            [func = std::move(func)](const CallHandle &handle) {
                if (traits::FuncObjTrait<T>::assertArgsNum(handle)) {
                    typename traits::FuncObjTrait<T>::ArgsTuple args_tuple;
                    argToTuple(handle.args(), args_tuple);
                    tryRun(
                        [&] {
                            if constexpr (traits::FuncObjTrait<
                                              T>::return_void) {
                                std::apply(func, args_tuple);
                                return ValAdaptor();
                            } else {
                                auto ret = std::apply(func, args_tuple);
                                return ret;
                            }
                        },
                        handle);
                }
            });
    }
    /*!
     * \brief 非同期に実行される関数をセットする
     * \since ver2.0
     *
     * * setAsync()でセットした場合、他クライアントから呼び出されたとき新しいスレッドを建てて実行される。
     * ver1.11以前のset()と同じ。
     * * 排他制御などはセットする関数の側で用意してください。
     *
     * \param func セットする関数または関数オブジェクト。
     * 引数はValAdaptorからキャスト可能な型ならいくつでも、
     * 戻り値はvoidまたはValAdaptorにキャスト可能な型が使用可能
     * \sa set()
     */
    template <typename T,
              typename traits::FuncObjTrait<
                  T>::ReturnTypeTrait::FuncReturnTypeCheckOk = traits::TraitOk,
              typename traits::FuncObjTrait<T>::ArgTypesTrait::ArgTypesCheckOk =
                  traits::TraitOk>
    const Func &setAsync(T func) const {
        return setImpl(
            valTypeOf<typename traits::FuncObjTrait<T>::ReturnType>(),
            traits::FuncObjTrait<T>::argsInfo(),
            [func_p = std::make_shared<T>(std::move(func))](
                const CallHandle &handle) {
                if (traits::FuncObjTrait<T>::assertArgsNum(handle)) {
                    typename traits::FuncObjTrait<T>::ArgsTuple args_tuple;
                    argToTuple(handle.args(), args_tuple);
                    std::thread(
                        [func_p, handle](auto args_tuple) {
                            tryRun(
                                [&] {
                                    if constexpr (traits::FuncObjTrait<
                                                      T>::return_void) {
                                        std::apply(*func_p, args_tuple);
                                        return ValAdaptor();
                                    } else {
                                        auto ret =
                                            std::apply(*func_p, args_tuple);
                                        return ret;
                                    }
                                },
                                handle);
                        },
                        std::move(args_tuple))
                        .detach();
                }
            });
    }
    /*!
     * \brief 関数をセットする
     *
     * set() と同じ。
     *
     * \deprecated ver2.0〜
     * set()とsetAsync()に分かれたので、代入演算子だとわかりづらい
     * また、 operator=(const Func &) との区別もつきづらい
     *
     */
    template <typename T>
    [[deprecated("use set() or setAsync()")]]
    const Func &operator=(T func) const {
        this->set(std::move(func));
        return *this;
    }

    /*!
     * \brief 引数にCallHandleを取る関数を登録する
     * \since ver1.9
     *
     * * WebCFace内部でCのAPIからの呼び出しに使うためのもの
     * * <del>ver1.11まで:
     * 関数がrespond()もreject()もせず終了した場合自動でrespondする。</del>
     * * ver2.0〜: 自動でrespondされることはないので、
     * 関数が受け取ったhandleを別スレッドに渡すなどして、
     * ここでセットした関数の終了後にrespond()やreject()することも可能。
     * * ver2.4〜: 例外をthrowした場合catchしてreject()する。
     *
     * \param args 引数の型などの情報
     * \param return_type 戻り値の型
     * \param func セットする関数または関数オブジェクト。
     * 引数としてCallHandleを1つ取り、戻り値はvoidで、
     * CallHandle::respond() や reject() を通して値を返す
     *
     */
    template <typename T,
              typename std::enable_if_t<
                  std::is_same_v<std::invoke_result_t<T, CallHandle>, void>,
                  std::nullptr_t> = nullptr>
    const Func &set(std::vector<Arg> args, ValType return_type,
                    T callback) const {
        auto args_size = args.size();
        return setImpl(return_type, std::move(args),
                       [args_size, callback = std::move(callback)](
                           const CallHandle &handle) {
                           if (handle.assertArgsNum(args_size)) {
                               catchAll([&] { callback(handle); }, handle);
                           }
                       });
    }
    /*!
     * \brief 引数にFuncCallHandleを取り非同期に実行される関数を登録する
     * \since ver2.0
     *
     * * WebCFace内部でCのAPIからの呼び出しに使うためのもの
     * * 関数がrespond()もreject()もせず終了した場合自動でrespondされることはないので、
     * 関数が受け取ったhandleを別スレッドに渡すなどして、
     * ここでセットした関数の終了後にrespond()やreject()することも可能。
     * * ver2.4〜: 例外をthrowした場合catchしてreject()する。
     *
     * \param args 引数の型などの情報
     * \param return_type 戻り値の型
     * \param func セットする関数または関数オブジェクト。
     * 引数としてCallHandleを1つ取り、戻り値はvoidで、
     * CallHandle::respond() や reject() を通して値を返す
     *
     */
    template <typename T,
              typename std::enable_if_t<
                  std::is_same_v<std::invoke_result_t<T, CallHandle>, void>,
                  std::nullptr_t> = nullptr>
    const Func &setAsync(std::vector<Arg> args, ValType return_type,
                         T callback) const {
        auto args_size = args.size();
        return setImpl(
            return_type, std::move(args),
            [args_size,
             callback = std::make_shared<std::function<void(FuncCallHandle)>>(
                 std::move(callback))](const CallHandle &handle) {
                if (handle.assertArgsNum(args_size)) {
                    std::thread([callback, handle] {
                        catchAll([&] { callback->operator()(handle); }, handle);
                    }).detach();
                }
            });
    }
    /*!
     * \brief 引数にCallHandleを取る関数を登録する
     * \since ver2.4
     *
     * * 自動でrespondされることはないので、
     * 関数が受け取ったhandleを別スレッドに渡すなどして、
     * ここでセットした関数の終了後にrespond()やreject()することも可能。
     * * setArgs(), setReturnType() で引数の個数や型と戻り値の型を指定する。
     * 指定しない場合、引数なし戻り値なしとみなされる。
     * * 例外をthrowした場合catchしてreject()する。
     *
     * \param func セットする関数または関数オブジェクト。
     * 引数としてCallHandleを1つ取り、戻り値はvoidで、
     * CallHandle::respond() や reject() を通して値を返す
     *
     */
    template <typename T,
              typename std::enable_if_t<
                  std::is_same_v<std::invoke_result_t<T, CallHandle>, void>,
                  std::nullptr_t> = nullptr>
    const Func &set(T callback) const {
        return setImpl(ValType::none_, std::nullopt,
                       [base = *this, callback = std::move(callback)](
                           const CallHandle &handle) {
                           if (handle.assertArgsNum(base.args().size())) {
                               catchAll([&] { callback(handle); }, handle);
                           }
                       });
    }
    /*!
     * \brief 引数にFuncCallHandleを取り非同期に実行される関数を登録する
     * \since ver2.4
     *
     * * 自動でrespondされることはないので、
     * 関数が受け取ったhandleを別スレッドに渡すなどして、
     * ここでセットした関数の終了後にrespond()やreject()することも可能。
     * * setArgs(), setReturnType() で引数の個数や型と戻り値の型を指定する。
     * 指定しない場合、引数なし戻り値なしとみなされる。
     * * 例外をthrowした場合catchしてreject()する。
     *
     * \param func セットする関数または関数オブジェクト。
     * 引数としてCallHandleを1つ取り、戻り値はvoidで、
     * CallHandle::respond() や reject() を通して値を返す
     *
     */
    template <typename T,
              typename std::enable_if_t<
                  std::is_same_v<std::invoke_result_t<T, CallHandle>, void>,
                  std::nullptr_t> = nullptr>
    const Func &setAsync(T callback) const {
        return setImpl(
            ValType::none_, std::nullopt,
            [base = *this,
             callback = std::make_shared<std::function<void(FuncCallHandle)>>(
                 std::move(callback))](const CallHandle &handle) {
                if (handle.assertArgsNum(base.args().size())) {
                    std::thread([callback, handle] {
                        catchAll([&] { callback->operator()(handle); }, handle);
                    }).detach();
                }
            });
    }

    /*!
     * \brief 関数を関数リストで非表示にする
     * (他clientのentryに表示されなくする)
     * \deprecated
     * ver1.10から、名前が半角ピリオドで始まるかどうかで判断されるように仕様変更したため、
     * hiddenの指定は無効 (この関数は効果がない)
     *
     */
    [[deprecated("Func::hidden() does nothing since ver1.10")]]
    const Func &hidden(bool) const {
        return *this;
    }

    /*!
     * \brief 関数の設定を削除
     *
     */
    const Func &free() const;

    /*!
     * \brief 関数を実行する (同期)
     *
     * 例外が発生した場合 runtime_error, 関数が存在しない場合 FuncNotFound
     * をthrowする
     *
     * \deprecated ver2.0〜 runAsync()を推奨。
     * Promise::waitFinish() と response(), rejection() で同等のことができるが、
     * 使い方によってはデッドロックを起こす可能性がある。
     * 詳細は waitFinish() のドキュメントを参照
     *
     */
    template <typename... Args>
    [[deprecated("use runAsync")]]
    ValAdaptor run(Args... args) const {
        return run({ValAdaptorVector(args)...});
    }
    [[deprecated("use runAsync")]]
    ValAdaptor run(std::vector<ValAdaptorVector> &&args_vec) const {
        auto p = runAsync(std::move(args_vec));
        p.waitFinish();
        if (p.found()) {
            if (p.isError()) {
                throw Rejection(*this, std::string(p.rejection()));
            } else {
                return p.response();
            }
        } else {
            throw FuncNotFound(*this);
        }
    }
    /*!
     * \brief run()と同じ
     * \deprecated ver2.0〜
     */
    template <typename... Args>
    [[deprecated("use runAsync")]]
    ValAdaptor operator()(Args... args) const {
        return run(args...);
    }

    /*!
     * \brief 関数を実行する (非同期)
     *
     * * 非同期で実行する。
     * 戻り値やエラー、例外は Promise から取得する
     * * 関数を実行したスレッドはdetachされるので、戻り値が不要な場合は
     * Promise を破棄してもよい。
     * (std::async などとは異なる)
     * * ver2.0～: runAsyncを呼んだ時点でclientがサーバーに接続していない場合、
     * 関数呼び出しメッセージは送信されず呼び出しは失敗する
     * (Promise::found() が false になる)
     *
     */
    template <typename... Args>
    Promise runAsync(Args... args) const {
        return runAsync({ValAdaptorVector(args)...});
    }
    Promise runAsync(std::vector<ValAdaptorVector> args_vec) const;

    /*!
     * \brief 関数の情報が存在すればtrue
     * \since ver2.1
     *
     */
    bool exists() const;
    /*!
     * \brief member内での関数の登録順番号
     * \since ver2.8
     *
     * 関数の情報が存在しない場合、また送信側のクライアントが古い場合0が返る
     *
     */
    int index() const;
    /*!
     * \brief indexを変更する
     * \since ver2.8
     *
     * * デフォルトではFuncがsetされた順に 1, 2, 3 ... とindexが割り振られるが、
     * これを手動で変更することにより、
     * WebUI等での表示順を変更することができる。
     * * 関数をsetした後に呼ばなければならない
     *   * set前に呼ぶと std::invalid_argument
     *
     */
    const Func &setIndex(int index) const;

    /*!
     * \brief 戻り値の型を返す
     *
     */
    ValType returnType() const;
    /*!
     * \brief 引数の情報を返す
     *
     * 変更するにはsetArgsを使う(このvectorの中身を書き換えても反映されない)
     *
     */
    std::vector<Arg> args() const;

    const Arg args(std::size_t i) const { return args().at(i); }

    /*!
     * \brief 引数の情報を更新する
     *
     * * setArgsで渡された引数の情報(名前など)とFuncがすでに持っている引数の情報(型など)がマージされる
     * * 関数のsetの後に呼ばなければならない
     *   * set前に呼ぶと std::invalid_argument
     *   * 例えば `Func(...).set(...).setArgs({...})` のようにする
     * * また、関数をセットしたあと cli.sync()
     * をする前に呼ばなければならない
     * * 実際にセットした関数の引数の数とargsの要素数は一致していなければならない
     *   * 一致していない場合 std::invalid_argument
     *
     */
    const Func &setArgs(const std::vector<Arg> &args) const;
    /*!
     * \brief 戻り値の型の情報を更新する
     * \since ver2.4
     *
     * * set()やsetAsync()で通常の関数をセットした場合戻り値の型は自動的に取得されるので
     * setReturnType() を呼ぶ必要はない。
     * * 関数をsetする前に呼ぶと std::invalid_argument
     *
     */
    const Func &setReturnType(ValType return_type) const;

    /*!
     * \brief Funcの参照先を比較
     * \since ver1.11
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Func>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Func>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};

class WEBCFACE_DLL FuncListener : protected Func {
    ValType return_type_ = ValType::none_;
    std::vector<Arg> args_{};

  public:
    FuncListener() = default;
    FuncListener(const Field &base);
    FuncListener(const Field &base, const SharedString &field)
        : FuncListener(Field{base, field}) {}

    friend class TemporalViewComponent;
    friend class TemporalCanvas2DComponent;
    using Field::member;
    using Field::name;

    /*!
     * \brief 関数呼び出しの待受を開始する
     *
     */
    FuncListener &listen();
    /*!
     * \brief 関数呼び出しの待受を開始する
     * \param args_num 引数の個数 (setArgsで設定していない場合)
     * \param return_type 戻り値の型 (setReturnTypeで設定していない場合)
     *
     */
    FuncListener &listen(std::size_t args_num,
                         ValType return_type = ValType::none_) {
        this->args_.resize(args_num);
        this->return_type_ = return_type;
        listen();
        return *this;
    }

    /*!
     * \brief 引数の情報をセットする
     *
     * listen() の前に呼ばなければならない。
     *
     */
    FuncListener &setArgs(const std::vector<Arg> &args) {
        this->args_ = args;
        return *this;
    }
    /*!
     * \brief 戻り値の型をセットする
     *
     * listen() の前に呼ばなければならない。
     *
     */
    FuncListener &setReturnType(ValType type) {
        this->return_type_ = type;
        return *this;
    }

    /*!
     * \brief 関数を関数リストで非表示にする
     * (他clientのentryに表示されなくする)
     * \deprecated
     * ver1.10から、名前が半角ピリオドで始まるかどうかで判断されるように仕様変更したため、
     * hiddenの指定は無効 (この関数は効果がない)
     *
     */
    [[deprecated("FuncListener::hidden() does nothing since ver1.10")]]
    FuncListener &hidden(bool) {
        return *this;
    }
    /*!
     * \brief 関数が呼び出されたかどうかを確認
     *
     * 1回の関数呼び出しに対してfetchCallは1回だけhandleを返す
     *
     * \return
     * 呼び出されたらその引数と、値を返す用の関数が入ったhandleを返す。
     * まだ呼び出されてなければnulloptを返す。
     *
     */
    std::optional<FuncCallHandle> fetchCall() const;
};

WEBCFACE_NS_END
