#pragma once
#include <vector>
#include "common/func.h"
#include "common/val.h"
#include "func_result.h"
#include "common/def.h"

namespace webcface {
namespace Internal {
struct ClientData;
}

namespace FuncWrapper {

//! Client::sync() まで待機し、実行完了までsync()をブロックするFuncWrapper
WEBCFACE_DLL FuncWrapperType
runCondOnSync(const std::weak_ptr<Internal::ClientData> &data);
//! ScopeGuardをロックするFuncWrapper
template <typename ScopeGuard>
inline FuncWrapperType runCondScopeGuard() {
    static auto wrapper = [](FuncType callback,
                             const std::vector<ValAdaptor> &args) {
        ScopeGuard scope_guard;
        return callback(args);
    };
    return wrapper;
}

} // namespace FuncWrapper

class AnonymousFunc;

//! 関数1つを表すクラス
class Func : protected Field {
  public:
    friend AnonymousFunc;

    Func() = default;
    WEBCFACE_DLL Func(const Field &base);
    Func(const Field &base, const std::string &field)
        : Func(Field{base, field}) {}

    using Field::member;
    using Field::name;

  private:
    WEBCFACE_DLL Func &setRaw(const std::shared_ptr<FuncInfo> &v);
    Func &setRaw(const FuncInfo &v) {
        return setRaw(std::make_shared<FuncInfo>(v));
    }
    WEBCFACE_DLL std::optional<std::shared_ptr<FuncInfo>> getRaw() const;
    WEBCFACE_DLL FuncWrapperType getDefaultFuncWrapper() const;

  public:
    //! 関数からFuncInfoを構築しセットする
    template <typename T>
    Func &set(const T &func) {
        return this->setRaw(
            FuncInfo{std::function{func}, getDefaultFuncWrapper()});
    }
    //! 関数からFuncInfoを構築しセットする
    template <typename T>
    Func &operator=(const T &func) {
        return this->set(func);
    }

    //! 関数を関数リストで非表示にする
    //! (他clientのentryに表示されなくする)
    WEBCFACE_DLL Func &hidden(bool hidden);

    //! 関数の設定を削除
    WEBCFACE_DLL Func &free();

    //! 関数を実行する (同期)
    /*! selfの関数の場合、このスレッドで直接実行する
     * 例外が発生した場合そのままthrow, 関数が存在しない場合 FuncNotFound
     * をthrowする
     *
     * リモートの場合、関数呼び出しを送信し結果が返ってくるまで待機
     * 例外が発生した場合 runtime_error, 関数が存在しない場合 FuncNotFound
     * をthrowする
     */
    template <typename... Args>
    ValAdaptor run(Args... args) const {
        return run({ValAdaptor(args)...});
    }
    WEBCFACE_DLL ValAdaptor run(const std::vector<ValAdaptor> &args_vec) const;
    //! run()と同じ
    template <typename... Args>
    ValAdaptor operator()(Args... args) const {
        return run(args...);
    }

    //! 関数を実行する (非同期)
    /*! 非同期で実行する
     * 戻り値やエラー、例外はAsyncFuncResultから取得する
     *
     * AsyncFuncResultはClient内部で保持されているのでClientが破棄されるまで参照は切れない
     */
    template <typename... Args>
    AsyncFuncResult &runAsync(Args... args) const {
        return runAsync({ValAdaptor(args)...});
    }
    WEBCFACE_DLL AsyncFuncResult &
    runAsync(const std::vector<ValAdaptor> &args_vec) const;

    //! 戻り値の型を返す
    WEBCFACE_DLL ValType returnType() const {
        auto func_info = getRaw();
        if (func_info) {
            return (*func_info)->return_type;
        }
        return ValType::none_;
    }
    //! 引数の情報を返す
    //! 変更するにはsetArgsを使う(このvectorの中身を書き換えても反映されない)
    WEBCFACE_DLL std::vector<Arg> args() const {
        auto func_info = getRaw();
        if (func_info) {
            return (*func_info)->args;
        }
        return std::vector<Arg>{};
    }
    const Arg args(std::size_t i) const { return args().at(i); }

    //! 引数の情報を更新する
    /*!
     * setArgsで渡された引数の情報(名前など)とFuncがすでに持っている引数の情報(型など)がマージされる
     *
     * 関数のセットの後に呼ばなければならない(セット前に呼ぶと
     * std::invalid_argument) (例えば Func(...).set(...).setArgs({...}) )
     * また、関数をセットしたあと cli.sync()
     * をする前に呼ばなければならない
     * 実際にセットした関数の引数の数とargsの要素数は一致していなければならない
     * (一致していない場合 std::invalid_argument )
     */
    WEBCFACE_DLL Func &setArgs(const std::vector<Arg> &args);

    /*! FuncWrapperをセットする。
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     * セットしない場合 Client::setDefaultRunCond() のものが使われる
     *
     * 関数のセットの後に呼ばなければならない(セット前に呼ぶと
     * std::invalid_argument)
     */
    WEBCFACE_DLL Func &setRunCond(FuncWrapperType wrapper);
    /*! FuncWrapperを nullptr にする
     */
    Func &setRunCondNone() { return setRunCond(nullptr); }
    /*! FuncWrapperを runCondOnSync() にする
     */
    Func &setRunCondOnSync() {
        return setRunCond(FuncWrapper::runCondOnSync(data_w));
    }
    /*! FuncWrapperを runCondScopeGuard() にする
     */
    template <typename ScopeGuard>
    Func &setRunCondScopeGuard() {
        return setRunCond(FuncWrapper::runCondScopeGuard<ScopeGuard>());
    }
};

//! 名前を指定せず先に関数を登録するFunc
class AnonymousFunc : public Func {
    WEBCFACE_DLL static std::string fieldNameTmp();

    std::function<void(AnonymousFunc &)> func_setter = nullptr;
    bool base_init = false;

  public:
    AnonymousFunc() = default;
    template <typename T>
    AnonymousFunc(const Field &base, const T &func)
        : Func(base, fieldNameTmp()), base_init(true) {
        this->set(func);
        this->hidden(true);
    }
    template <typename T>
    AnonymousFunc(const T &func) {
        func_setter = [func](AnonymousFunc &a) {
            a.set(func);
            a.hidden(true);
        };
    }
    
    //! targetに関数を移動
    WEBCFACE_DLL void lockTo(Func &target);
};
} // namespace webcface
