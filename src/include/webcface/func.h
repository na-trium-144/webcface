#pragma once
#include <vector>
#include "common/func.h"
#include "common/val.h"
#include "func_result.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN

namespace FuncWrapper {

/*!
 * \brief Client::sync() まで待機し、実行完了までsync()をブロックするFuncWrapper
 *
 */
WEBCFACE_DLL FuncWrapperType
runCondOnSync(const std::weak_ptr<Internal::ClientData> &data);
/*!
 * \brief ScopeGuardをロックするFuncWrapper
 *
 */
template <typename ScopeGuard>
inline FuncWrapperType runCondScopeGuard() {
    static auto wrapper = [](const FuncType &callback,
                             const std::vector<ValAdaptor> &args) {
        ScopeGuard scope_guard;
        return callback(args);
    };
    return wrapper;
}

} // namespace FuncWrapper

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
    Func(const Field &base, std::u8string_view field)
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
     * \since ver1.12
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
     * \since ver1.12
     */
    Func operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Func operator[](const char *field) const { return child(field); }
    /*!
     * \since ver1.12
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
    Func &setRaw(const std::shared_ptr<FuncInfo> &v);
    Func &setRaw(const FuncInfo &v) {
        return setRaw(std::make_shared<FuncInfo>(v));
    }
    FuncWrapperType getDefaultFuncWrapper() const;

    void runImpl(std::size_t caller_id,
                 const std::vector<ValAdaptor> &args_vec) const;

  public:
    /*!
     * \brief 関数からFuncInfoを構築しセットする
     *
     */
    template <typename T>
    Func &set(const T &func) {
        return this->setRaw(
            FuncInfo{std::function{func}, getDefaultFuncWrapper()});
    }
    /*!
     * \brief 関数からFuncInfoを構築しセットする
     *
     */
    template <typename T>
    Func &operator=(const T &func) {
        return this->set(func);
    }

    /*!
     * \brief 引数にFuncCallHandleを取る関数を登録する
     * \since ver1.9
     *
     * cからの呼び出し用
     *
     */
    Func &set(const std::vector<Arg> &args, ValType return_type,
              const std::function<void(FuncCallHandle)> &callback);

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
     * 非同期で実行する
     * 戻り値やエラー、例外はAsyncFuncResultから取得する
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
     * \brief FuncWrapperをセットする。
     *
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     * セットしない場合 Client::setDefaultRunCond() のものが使われる
     *
     * 関数のセットの後に呼ばなければならない(セット前に呼ぶと
     * std::invalid_argument)
     *
     */
    Func &setRunCond(const FuncWrapperType &wrapper);
    /*!
     * \brief FuncWrapperを nullptr にする
     *
     */
    Func &setRunCondNone() { return setRunCond(nullptr); }
    /*!
     * \brief FuncWrapperを runCondOnSync() にする
     *
     */
    Func &setRunCondOnSync() {
        return setRunCond(FuncWrapper::runCondOnSync(data_w));
    }
    /*!
     * \brief FuncWrapperを runCondScopeGuard() にする
     *
     */
    template <typename ScopeGuard>
    Func &setRunCondScopeGuard() {
        return setRunCond(FuncWrapper::runCondScopeGuard<ScopeGuard>());
    }

    /*!
     * \brief Funcの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, Func>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};

/*!
 * \brief 名前を指定せず先に関数を登録するFunc
 *
 */
class WEBCFACE_DLL AnonymousFunc : public Func {
    static std::u8string fieldNameTmp();

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
    AnonymousFunc(const Field &base, const T &func)
        : Func(base, fieldNameTmp()), base_init(true) {
        this->set(func);
    }
    /*!
     * コンストラクタでdataが渡されなかった場合は関数を内部で保持し(func_setter)、
     * lockTmp() 時にdataに登録する
     *
     */
    template <typename T>
    explicit AnonymousFunc(const T &func)
        : func_setter([func](AnonymousFunc &a) { a.set(func); }) {}

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
    FuncListener(const Field &base, std::u8string_view field)
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
