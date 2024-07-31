#pragma once
#include <vector>
#include "func_info.h"
#include "webcface/encoding/val_adaptor.h"
#include "func_result.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief 関数1つを表すクラス
 *
 */
class WEBCFACE_DLL Func : protected Field {
  public:
    friend class AnonymousFunc;
    friend class ViewComponent;
    friend class Canvas2DComponent;

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

  protected:
    /*!
     * set2()で構築された関数の情報(FuncInfo)をclientにセット
     *
     */
    Func &setImpl(const std::shared_ptr<FuncInfo> &v);

    template <typename... Args>
    using ArgsTuple = std::tuple<std::decay_t<Args>...>;

    /*!
     * 関数の引数をvectorで受け取りtupleに変換する。
     * \param eval_async
     * futureがdeferであり関数の結果を取得するのをasyncで実行するべきならtrue,
     * 結果のfutureが即座に返るので非同期にする意味がないならfalse
     * \param func_casted
     * set1()で引数をtuple、戻り値をfutureにした、これからセットする関数
     *
     */
    template <typename Ret, typename... Args>
    Func &
    set2(bool eval_async,
         std::function<std::future<ValAdaptor>(const ArgsTuple<Args...> &)>
             func_casted) {
        return setImpl(std::make_shared<FuncInfo>(
            valTypeOf<Ret>(), std::vector<Arg>{Arg{valTypeOf<Args>()}...},
            eval_async,
            [func_casted = std::move(func_casted)](
                const std::vector<ValAdaptor> &args_vec) {
                ArgsTuple<Args...> args_tuple;
                if (args_vec.size() != sizeof...(Args)) {
                    throw std::invalid_argument(
                        "requires " + std::to_string(sizeof...(Args)) +
                        " arguments, got " + std::to_string(args_vec.size()));
                }
                argToTuple(args_vec, args_tuple);
                return func_casted(args_tuple);
            }));
    }

    /*!
     * set()に渡された関数が戻り値がshared_futureの関数の場合:
     * 引数をtuple型で受け取り、関数をそのまま実行し、
     * shared_futureから戻り値を取得する処理をdefer状態でfutureに格納して返す関数を
     * set2()に渡す
     *
     */
    template <typename Ret, typename... Args>
    Func &set1(std::function<std::shared_future<Ret>(Args...)> func) {
        return set2<Ret, Args...>(
            true,
            [func = std::move(func)](const ArgsTuple<Args...> &args_tuple) {
                auto ret = std::apply(func, args_tuple);
                return std::async(
                    std::launch::deferred,
                    [](std::shared_future<Ret> ret) {
                        if constexpr (std::is_void_v<Ret>) {
                            ret.get();
                            return ValAdaptor{};
                        } else {
                            return static_cast<ValAdaptor>(ret.get());
                        }
                    },
                    std::move(ret));
            });
    }
    /*!
     * set()に渡された関数が戻り値がfutureの関数の場合:
     * 引数をtuple型で受け取り、関数をそのまま実行し、
     * futureから戻り値を取得する処理をdefer状態でfutureに格納して返す関数を
     * set2()に渡す
     *
     */
    template <typename Ret, typename... Args>
    Func &set1(std::function<std::future<Ret>(Args...)> func) {
        return set2<Ret, Args...>(
            true,
            [func = std::move(func)](const ArgsTuple<Args...> &args_tuple) {
                auto ret = std::apply(func, args_tuple);
                return std::async(
                    std::launch::deferred,
                    [](std::future<Ret> ret) {
                        if constexpr (std::is_void_v<Ret>) {
                            ret.get();
                            return ValAdaptor{};
                        } else {
                            return static_cast<ValAdaptor>(ret.get());
                        }
                    },
                    std::move(ret));
            });
    }
    /*!
     * set()に渡された関数が戻り値が値の普通の関数の場合:
     * 引数をtuple型で受け取り、関数をそのまま実行し、戻り値をfuture<ValAdaptor>型で返す関数を
     * set2()に渡す
     *
     */
    template <typename Ret, typename... Args>
    Func &set1(std::function<Ret(Args...)> func) {
        return set2<Ret, Args...>(
            false,
            [func = std::move(func)](const ArgsTuple<Args...> &args_tuple) {
                std::packaged_task<ValAdaptor()> task([&] {
                    if constexpr (std::is_void_v<Ret>) {
                        std::apply(func, args_tuple);
                        return ValAdaptor{};
                    } else {
                        Ret ret = std::apply(func, args_tuple);
                        return static_cast<ValAdaptor>(ret);
                    }
                });
                task();
                return task.get_future();
            });
    }
    /*!
     * setAsync()に渡された関数が戻り値が値の普通の関数の場合:
     * 引数をtuple型で受け取り、関数をdefer状態でfutureに格納して返す関数を
     * set2()に渡す
     *
     */
    template <typename Ret, typename... Args>
    Func &setAsync1(std::function<Ret(Args...)> func) {
        return set2<Ret, Args...>(
            true, [func = std::make_shared<std::function<Ret(Args...)>>(
                       std::move(func))](ArgsTuple<Args...> args_tuple) {
                return std::async(
                    std::launch::deferred,
                    [func](ArgsTuple<Args...> args_tuple) {
                        if constexpr (std::is_void_v<Ret>) {
                            std::apply(*func, args_tuple);
                            return ValAdaptor{};
                        } else {
                            Ret ret = std::apply(*func, args_tuple);
                            return static_cast<ValAdaptor>(ret);
                        }
                    },
                    std::move(args_tuple));
            });
    }

    void runImpl(std::size_t caller_id, std::vector<ValAdaptor> args_vec,
                 bool caller_async) const;

  public:
    /*!
     * \brief 関数をセットする
     *
     * * 引数はValAdaptorからキャスト可能な型ならいくつでも、
     * 戻り値はvoidまたはValAdaptorにキャスト可能な型または
     * (ver2.0〜: std::futureまたはstd::shared_future)
     * が使用可能
     * * (ver2.0〜) set()でセットした場合、他クライアントから呼び出されたとき
     * Client::recv() (または autoRecv) のスレッドでそのまま実行され、
     * この関数が完了するまで他のデータの受信はブロックされる。
     * また、 runAsync() で呼び出したとしても同じスレッドで同期実行される。
     * * ただし関数がstd::futureまたはstd::shared_futureを返す場合、
     * そのfutureの評価(future.get())は新しく建てたスレッドで行われる。
     *
     * \sa setAsync()
     */
    template <typename T>
    Func &set(T &&func) {
        return this->set1(std::function{std::forward<T>(func)});
    }
    /*!
     * \brief 非同期に実行される関数をセットする
     * \since ver2.0
     *
     * * setAsync()でセットした場合、他クライアントから呼び出されたとき新しいスレッドを建てて実行される。
     * ver1.11以前のset()と同じ。
     * また、`set([](args...){ return std::async(std::launch::deferred, func,
     * args...); })` と同じ。
     * * 排他制御などはセットする関数の側で用意してください。
     *
     * \sa set()
     */
    template <typename T>
    Func &setAsync(T &&func) {
        return this->setAsync1(std::function{std::forward<T>(func)});
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
    [[deprecated("use set() or setAsync()")]] Func &operator=(T func) {
        this->set(std::move(func));
        return *this;
    }

    /*!
     * \brief 引数にFuncCallHandleを取る関数を登録する
     * \since ver1.9
     *
     * cからの呼び出し用
     *
     */
    Func &set(const std::vector<Arg> &args, ValType return_type,
              std::function<void(FuncCallHandle)> callback);
    /*!
     * \brief 引数にFuncCallHandleを取り非同期に実行される関数を登録する
     * \since ver2.0
     *
     * cからの呼び出し用
     *
     */
    Func &setAsync(const std::vector<Arg> &args, ValType return_type,
                   std::function<void(FuncCallHandle)> callback);

    /*!
     * \brief 関数を関数リストで非表示にする
     * (他clientのentryに表示されなくする)
     * \deprecated
     * ver1.10から、名前が半角ピリオドで始まるかどうかで判断されるように仕様変更したため、
     * hiddenの指定は無効 (この関数は効果がない)
     *
     */
    [[deprecated("Func::hidden() does nothing since ver1.10")]] Func &
    hidden(bool) {
        return *this;
    }

    /*!
     * \brief 関数の設定を削除
     *
     */
    Func &free();

    /*!
     * \brief 関数を実行する (同期)
     *
     * selfの関数の場合、このスレッドで直接実行する
     * 例外が発生した場合そのままthrow, 関数が存在しない場合 FuncNotFound
     * をthrowする
     *
     * リモートの場合、関数呼び出しを送信し結果が返ってくるまで待機
     * 例外が発生した場合 runtime_error, 関数が存在しない場合 FuncNotFound
     * をthrowする
     *
     */
    template <typename... Args>
    ValAdaptor run(Args... args) const {
        return run({ValAdaptor(args)...});
    }
    ValAdaptor run(const std::vector<ValAdaptor> &args_vec) const;
    /*!
     * \brief run()と同じ
     *
     */
    template <typename... Args>
    ValAdaptor operator()(Args... args) const {
        return run(args...);
    }

    /*!
     * \brief 関数を実行する (非同期)
     *
     * * 非同期で実行する。
     * 戻り値やエラー、例外は AsyncFuncResult から取得する
     * * 関数を実行したスレッドはdetachされるので、戻り値が不要な場合は
     * AsyncFuncResult を破棄してもよい。
     * (std::async とは異なる)
     *
     */
    template <typename... Args>
    AsyncFuncResult runAsync(Args... args) const {
        return runAsync({ValAdaptor(args)...});
    }
    AsyncFuncResult runAsync(const std::vector<ValAdaptor> &args_vec) const;

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
    Func &setArgs(const std::vector<Arg> &args);

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
    template <typename T>
    AnonymousFunc(const Field &base, T &&func)
        : Func(base, fieldNameTmp()), base_init(true) {
        this->set(std::forward<T>(func));
    }
    /*!
     * コンストラクタでdataが渡されなかった場合は関数を内部で保持し(func_setter)、
     * lockTmp() 時にdataに登録する
     *
     */
    template <typename T>
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
    [[deprecated(
        "FuncListener::hidden() does nothing since ver1.10")]] FuncListener &
    hidden(bool) {
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
