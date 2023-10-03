# Func, AnonymousFunc

API Reference → WebCFace::Func

別クライアントから呼び出せる関数の登録をします。

Member::func() でFuncクラスのオブジェクトが得られます
```cpp
WebCFace::Func func_hoge = wcli.member("a").func("hoge");
```

Member::funcs() でそのMemberが送信しているfuncのリストが得られます
```cpp
for(const WebCFace::Func &v: wcli.member("a").funcs()){
	// ...
}
```

Member::onFuncEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onFuncEntry().appendListener([](WebCFace::Func v){ /* ... */ });
```

## 関数の登録

自分自身の名前のMemberからFuncオブジェクトを作り、 Func::set() で関数を代入してください。
関数はstd::functionに変換できるものであればなんでもokです。

引数、戻り値はint, double, bool, std::string型であればいくつでも自由に指定できます。
```cpp
wcli.func("hoge").set([](){ /* ... */ });
wcli.func("fuga").set([](int a, const std::string &b){ return 3.1415; });
```

set() の代わりに代入演算子(Value::operator=)でも同様のことができます

関数の設定後 wcli.sync() することで他のクライアントからその関数の情報を見ることができるようになります。

### 引数

Func::setArgs() で引数名などの情報をセットすることができます。
ただし関数をセットする前に呼ぶとエラーになります。
また、実際の関数の引数と個数が一致していなければエラーになります。
```cpp
wcli.func("fuga").setArgs(){
	WebCFace::Arg("a").init(100),
	WebCFace::Arg("b").option({"aaa", "bbb", "ccc"}),
}
```

他のクライアントの関数の情報は Func::returnType() や Func::args() で取得できます。

### 実行条件

デフォルトでは、呼び出された関数は別スレッドで非同期に実行されます。
これをメインスレッドと同期させたい場合は実行条件を設定することができます。
```cpp
wcli.func("fuga").setRunCondOnSync();
```
とすると呼び出された関数の実行は wcli.sync() のときに行われます。

```cpp
wcli.func("fuga").setRunCondScopeGuard<ScopeGuard>();
```
とすると任意のScopeGuardクラスを使うことができます(実行前にScopeGuardのコンストラクタ、実行後にデストラクタが呼ばれます)

また、すべての関数にまとめて条件を設定したい場合は、関数の設定前に
```cpp
wcli.setDefaultRunCondOnSync();
wcli.setDefaultRunCondScopeGuard<ScopeGuard>();
```
とするとデフォルトの条件を設定できます。

デフォルトを設定した後個別の関数について条件を設定することもできますし、
```cpp
wcli.func("fuga").setRunCondNone();
```
で条件を何も課さないようにできます。

## 関数の実行

Func::run() で関数を実行できます。引数を渡すこともでき、戻り値もそのまま返ってきます。
他クライアントの関数も同様にrun()で実行させることができます。

実行した関数が例外を投げた場合、また引数の個数が一致しない場合などはrun()が例外を投げます。
```cpp
double ans = wcli.member("a").func("hoge").run(1, "aa");
```
operator() でも同様に実行できます。

Func::runAsync() は関数の実行を開始し、終了を待たずに続行します。
戻り値として AsyncFuncResult クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。
```cpp
AsyncFuncResult res = wcli.member("a").func("hoge").runAsync(1, "aa");
double ans = res.result.get();
```

## AnonymousFunc

名前を指定しないFuncです。
[Viewのボタン](./13_view.md#button)などのコールバックに関数を設定したい場合に使うことができます。

wcli.func() で取得できるようにする予定だが未実装。(現在はViewComponentの内部でのみ使われている)
