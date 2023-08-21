#pragma once
#include <vector>
#include "data.h"
#include "common/func.h"
#include "common/val.h"
#include "func_result.h"

namespace WebCFace {

namespace FuncWrapper {

//! Client::sync() まで待機し、実行完了までsync()をブロックするFuncWrapper
FuncWrapperType runCondOnSync(const std::weak_ptr<ClientData> &data);
//! ScopeGuardをロックするFuncWrapper
template <typename ScopeGuard>
FuncWrapperType runCondScopeGuard() {
    static auto wrapper = [](FuncType callback,
                             const std::vector<ValAdaptor> &args) {
        ScopeGuard scope_guard;
        return callback(args);
    };
    return wrapper;
}

} // namespace FuncWrapper

//! 関数1つを表すクラス
class Func : public SyncFieldBase<FuncInfo> {
  public:
    Func() = default;
    Func(const FieldBase &base) : SyncFieldBase<FuncInfo>(base) {}
    Func(const FieldBase &base, const std::string &field)
        : Func(FieldBase{base, field}) {}

    auto &set(const FuncInfo &v) {
        setCheck();
        dataLock()->func_store.setSend(*this, v);
        return *this;
    }
    //! 関数からFuncInfoを構築しセットする
    /*! Tは任意の関数
     * 一度セットしたFuncに別の関数をセットすると、それ以降実行される関数は新しい関数になるが、
     * 引数や戻り値の情報などは更新されない
     */
    template <typename T>
    auto &set(const T &func) {
        return this->set(
            FuncInfo{std::function{func}, dataLock()->default_func_wrapper});
    }
    //! 関数からFuncInfoを構築しセットする
    template <typename T>
    Func &operator=(const T &func) {
        return this->set(func);
    }

    //! 値を取得する
    std::optional<FuncInfo> tryGet() const override {
        return dataLock()->func_store.getRecv(*this);
    }

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
    ValAdaptor run(const std::vector<ValAdaptor> &args_vec) const;
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
    AsyncFuncResult &runAsync(const std::vector<ValAdaptor> &args_vec) const;

    //! 戻り値の型を返す
    ValType returnType() const;
    //! 引数の情報を返す
    //! 変更するにはsetArgsを使う(このvectorの中身を書き換えても反映されない)
    std::vector<Arg> args() const;
    //! 引数の情報を更新する
    /*!
     * setArgsで渡された引数の情報(名前など)とFuncがすでに持っている引数の情報(型など)がマージされる
     *
     * 関数のセットの後に呼ばなければならない (例えば
     * Func(...).set(...).setArgs({...}) ) 関数をしたあと cli.sync()
     * をする前に呼ばなければならない
     * 実際にセットした関数の引数の数とargsの要素数は一致していなければならない
     */
    Func &setArgs(const std::vector<Arg> &args);

    /*! FuncWrapperをセットする。
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     * セットしない場合 Client::setDefaultRunCond() のものが使われる
     *
     * 関数のセットの後に呼ばなければならない
     */
    Func &setRunCond(FuncWrapperType wrapper);
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


} // namespace WebCFace