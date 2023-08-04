#pragma once
#include <string>
#include <istream>
#include <ostream>
#include <optional>
#include "data_store.h"
#include "decl.h"

namespace WebCFace {

//! データの参照先を表すクラス。
/*! SyncDataとSyncDataStoreの関数定義はヘッダーに書いてないが、
 * ソース(client/data.cc)に書いて、特定の型についてインスタンス化する
 */
template <typename T>
class SyncData {
  protected:
    Client *cli;
    //! cliが持っているstoreを表すがTによって参照先が違う
    SyncDataStore<T> *store;

    std::string member_, name_;

  public:
    using DataType = T;
    SyncData() {}
    SyncData(Client *cli, const std::string &member, const std::string &name);
    //! 参照先のMemberを返す
    Member member() const;
    //! 参照先のデータ名を返す
    std::string name() const { return name_; }

    //! 値をセットする
    virtual void set(const T &data);
    //! 値を取得する
    std::optional<T> try_get() const;
    //! 値を取得する
    //! todo: 引数
    T get() const;
    //! 値を取得する
    operator T() const { return this->get(); }
};

template <typename T>
auto &operator>>(std::basic_istream<char> &is, SyncData<T> &data) {
    T v;
    is >> v;
    data.set(v);
    return is;
}
template <typename T>
auto &operator>>(std::basic_istream<char> &is, SyncData<T> &&data) {
    SyncData<T> d = data;
    return is >> d;
}
template <typename T>
auto &operator<<(std::basic_ostream<char> &os, const SyncData<T> &data) {
    return os << data.get();
}

} // namespace WebCFace
