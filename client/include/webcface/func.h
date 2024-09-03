#pragma once
#include "webcface/encoding/val_adaptor.h"
#include "func_result.h"
#include "arg.h"

WEBCFACE_NS_BEGIN
namespace internal {
struct FuncInfo;
}

template <typename T>
constexpr auto getInvokeSignature(T &&) -> decltype(&T::operator()) {
    return &T::operator();
}
template <typename Ret, typename... Args>
constexpr auto getInvokeSignature(Ret (*p)(Args...)) {
    return p;
}
template <typename T>
using InvokeSignature =
    decltype(getInvokeSignature(std::declval<std::decay_t<T>>()));

template <bool>
struct FuncArgTypeCheck {};
template <>
struct FuncArgTypeCheck<true> {
    using ArgTypesSupportedByWebCFaceFunc = std::nullptr_t;
};
template <typename... Args>
struct FuncArgTypesTrait
    : FuncArgTypeCheck<(std::is_convertible_v<ValAdaptor, Args> && ...)> {};

template <bool>
struct FuncReturnTypeCheck {};
template <>
struct FuncReturnTypeCheck<true> {
    using ReturnTypeSupportedByWebCFaceFunc = std::nullptr_t;
};
template <typename Ret>
struct FuncReturnTypeTrait
    : FuncReturnTypeCheck<std::is_same_v<Ret, void> ||
                          std::is_constructible_v<ValAdaptor, Ret>> {};

template <typename T>
struct FuncSignatureTrait {};
/*!
 * RetとArgsが条件を満たすときだけ、
 * ReturnTypeTrait::ReturnTypeSupportedByWebCFaceFunc と
 * ArgTypesTrait::ArgTypesSupportedByWebCFaceFunc が定義される
 * (enable_ifを使ってないのはエラーメッセージがわかりにくかったから)
 *
 */
template <typename Ret, typename... Args>
struct FuncSignatureTrait<Ret(Args...)> {
    using ReturnTypeTrait = FuncReturnTypeTrait<Ret>;
    using ArgTypesTrait = FuncArgTypesTrait<Args...>;
    static constexpr bool return_void = std::is_same_v<Ret, void>;
    static inline bool assertArgsNum(const CallHandle &handle) {
        return handle.assertArgsNum(sizeof...(Args));
    }
    static inline std::vector<Arg> argsInfo() {
        return std::vector<Arg>{Arg{valTypeOf<Args>()}...};
    }
    using ReturnType = Ret;
    using ArgsTuple = std::tuple<std::decay_t<Args>...>;
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

/*!
 * * Tは関数オブジェクト型、または関数型
 * * InvokeSignature<T> で関数呼び出しの型 ( Ret(*)(Args...) や
 * Ret(T::*)(Args...) ) を取得し、
 * * FuncSignatureTrait<Ret(Args...)> が各種メンバー型や定数を定義する
 *
 */
template <typename T>
using FuncObjTrait = FuncSignatureTrait<InvokeSignature<T>>;

/*!
 * \brief 関数1つを表すクラス
 *
 */
class WEBCFACE_DLL Func : protected Field {
  public:
    friend class AnonymousFunc;
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
     */
    Func child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Func child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Func child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Func operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    Func operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Func operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    Func operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Func operator[](int index) const { return child(index); }
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
    const Func &
    setImpl(const std::shared_ptr<internal::FuncInfo> &func_info) const;

    /*!
     * f_run()を実行し結果をCallHandleに渡す
     *
     */
    template <typename F1>
    static void tryRun(F1 &&f_run, const CallHandle &handle) {
        ValAdaptor error;
        try {
            handle.respond(f_run());
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

    static constexpr std::nullptr_t TraitOk = nullptr;

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
              typename FuncObjTrait<T>::ReturnTypeTrait::
                  ReturnTypeSupportedByWebCFaceFunc = TraitOk,
              typename FuncObjTrait<
                  T>::ArgTypesTrait::ArgTypesSupportedByWebCFaceFunc = TraitOk>
    const Func &set(T func) const {
        return setImpl(
            valTypeOf<typename FuncObjTrait<T>::ReturnType>(),
            FuncObjTrait<T>::argsInfo(),
            [func = std::move(func)](const CallHandle &handle) {
                if (FuncObjTrait<T>::assertArgsNum(handle)) {
                    typename FuncObjTrait<T>::ArgsTuple args_tuple;
                    argToTuple(handle.args(), args_tuple);
                    tryRun(
                        [&] {
                            if constexpr (FuncObjTrait<T>::return_void) {
                                std::apply(func, args_tuple);
                                return ValAdaptor::emptyVal();
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
              typename FuncObjTrait<T>::ReturnTypeTrait::
                  ReturnTypeSupportedByWebCFaceFunc = TraitOk,
              typename FuncObjTrait<
                  T>::ArgTypesTrait::ArgTypesSupportedByWebCFaceFunc = TraitOk>
    const Func &setAsync(T func) const {
        return setImpl(
            valTypeOf<typename FuncObjTrait<T>::ReturnType>(),
            FuncObjTrait<T>::argsInfo(),
            [func_p = std::make_shared<T>(std::move(func))](
                const CallHandle &handle) {
                if (FuncObjTrait<T>::assertArgsNum(handle)) {
                    typename FuncObjTrait<T>::ArgsTuple args_tuple;
                    argToTuple(handle.args(), args_tuple);
                    std::thread(
                        [func_p, handle](auto args_tuple) {
                            tryRun(
                                [&] {
                                    if constexpr (FuncObjTrait<
                                                      T>::return_void) {
                                        std::apply(*func_p, args_tuple);
                                        return ValAdaptor::emptyVal();
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
    [[deprecated("use set() or setAsync()")]] const Func &
    operator=(T func) const {
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
                               callback(handle);
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
                        callback->operator()(handle);
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
        return run({ValAdaptor(args)...});
    }
    [[deprecated("use runAsync")]]
    ValAdaptor run(std::vector<ValAdaptor> &&args_vec) const {
        auto p = runAsync(std::move(args_vec));
        p.waitFinish();
        if (p.found()) {
            if (p.isError()) {
                throw std::runtime_error(p.rejection());
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
        return runAsync({ValAdaptor(args)...});
    }
    Promise runAsync(std::vector<ValAdaptor> args_vec) const;

    /*!
     * \brief 関数の情報が存在すればtrue
     * \since ver2.1
     *
     */
    bool exists() const;

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
     * setArgsで渡された引数の情報(名前など)とFuncがすでに持っている引数の情報(型など)がマージされる
     *
     * 関数のセットの後に呼ばなければならない(セット前に呼ぶと
     * std::invalid_argument) (例えば Func(...).set(...).setArgs({...}) )
     * また、関数をセットしたあと cli.sync()
     * をする前に呼ばなければならない
     * 実際にセットした関数の引数の数とargsの要素数は一致していなければならない
     * (一致していない場合 std::invalid_argument )
     *
     */
    const Func &setArgs(const std::vector<Arg> &args) const;

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

/*!
 * \brief 名前を指定せず先に関数を登録するFunc
 *
 */
class WEBCFACE_DLL AnonymousFunc : public Func {
    static SharedString WEBCFACE_CALL fieldNameTmp();

    std::function<void(AnonymousFunc &)> func_setter = nullptr;
    bool base_init = false;

  public:
    AnonymousFunc() = default;
    /*!
     * 一時的な名前(fieldNameTmp())をつけたFuncとしてdataに登録し、
     * lockTmp() 呼び出し時に正式な名前のFuncに内容を移動する。
     *
     */
    template <typename T,
              typename FuncObjTrait<T>::ReturnTypeTrait::
                  ReturnTypeSupportedByWebCFaceFunc = TraitOk,
              typename FuncObjTrait<
                  T>::ArgTypesTrait::ArgTypesSupportedByWebCFaceFunc = TraitOk>
    AnonymousFunc(const Field &base, T func)
        : Func(base, fieldNameTmp()), base_init(true) {
        this->set(std::move(func));
    }
    /*!
     * コンストラクタでdataが渡されなかった場合は関数を内部で保持し(func_setter)、
     * lockTmp() 時にdataに登録する
     *
     */
    template <typename T,
              typename FuncObjTrait<T>::ReturnTypeTrait::
                  ReturnTypeSupportedByWebCFaceFunc = TraitOk,
              typename FuncObjTrait<
                  T>::ArgTypesTrait::ArgTypesSupportedByWebCFaceFunc = TraitOk>
    explicit AnonymousFunc(T func)
        : func_setter([func = std::move(func)](AnonymousFunc &a) mutable {
              a.set(std::move(func));
          }) {}

    AnonymousFunc(const AnonymousFunc &) = delete;
    AnonymousFunc &operator=(const AnonymousFunc &) = delete;
    AnonymousFunc(AnonymousFunc &&other) noexcept { *this = std::move(other); }
    /*!
     * \brief otherの中身を移動し、otherは未初期化にする
     * \since ver1.9
     *
     * 未初期化 == func_setterが空でbase_initがfalse
     *
     */
    AnonymousFunc &operator=(AnonymousFunc &&other) noexcept;
    /*!
     * \brief targetに関数を移動
     *
     * (ver1.9〜) thisが未初期化の場合 std::runtime_error を投げる
     *
     * (ver1.9〜) 2回実行すると std::runtime_error
     *
     */
    void lockTo(Func &target);
};

class WEBCFACE_DLL FuncListener : protected Func {
    ValType return_type_ = ValType::none_;
    std::vector<Arg> args_{};

  public:
    FuncListener() = default;
    FuncListener(const Field &base);
    FuncListener(const Field &base, const SharedString &field)
        : FuncListener(Field{base, field}) {}

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
