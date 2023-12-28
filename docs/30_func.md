# Func

API Reference → webcface::Func, webcface::AnonymousFunc

他Memberから呼び出せる関数の登録、
また他Memberの関数の呼び出しができます。

Member::func() でFuncクラスのオブジェクトが得られます
```cpp
webcface::Func func_hoge = wcli.member("a").func("hoge");
```

Member::funcs() でそのMemberが送信しているfuncのリストが得られます
```cpp
for(const webcface::Func &v: wcli.member("a").funcs()){
	// ...
}
```

Member::onFuncEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onFuncEntry().appendListener([](webcface::Func v){ /* ... */ });
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

Pythonでは引数と戻り値のアノテーションをすると自動で取得されます。
```py
def hoge(a: int, b: str) -> float:
	return 3.1415

wcli.func("hoge").set(hoge)
wcli.func("lambda").set(lambda x: return x + 5) # ラムダ式なども可
```
また、setを明示的に呼び出す代わりにfuncオブジェクトをデコレータにすると簡単に登録できます。
この場合func()の引数に関数名を書くのを省略すると実際の関数の名前が自動で取得され設定されます。
(明示的に関数名を渡しても良い)
```py
@wcli.func()
def hoge(a: int, b: str) -> float:
	return 3.1415
```

同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。

### 引数

Func::setArgs() で引数名などの情報や、引数に設定する条件をセットすることができます。
型情報を取得できないJavaScript(TypeScript含む)の場合と、Pythonでアノテーションをしなかった場合はここで型情報も指定する必要があります。
設定可能な情報の一覧は webcface::Arg を参照

C++では Func.setArgs() で指定できます。
関数をセットする前に呼ぶとエラーになります。
また、実際の関数の引数と個数が一致していなければエラーになります。
```cpp
wcli.func("fuga").setArgs(){
	webcface::Arg("a").init(100),
	webcface::Arg("b").option({"aaa", "bbb", "ccc"}),
}
```

Pythonではset()関数の引数(デコレータとして使う場合はfunc()の引数)にオプションで args と return_type を渡すことで指定できます。
関数に型アノテーションがある場合はここで型の指定は不要です。
また引数名も自動で取得されるので不要ですが明示的に指定することもできます。
```py
wcli.func("hoge").set(hoge, return_type=float, args=[
	Arg(type=int, init=100),
	Arg(type=str, option=["aaa", "bbb", "ccc"]),
])
```

JavaScriptでも同様にset関数の引数で指定します。
```js
wcli.func("hoge").set(hoge, valType.float_, [
	{ name: "a", type: valType.int_, init: 100 },
	{ name: "b", type: valType.string_, option: ["aaa", "bbb", "ccc"] },
]);
```

他のクライアントの関数の情報は Func::returnType() や Func::args() で取得できます。

### hidden属性

関数の設定後 Client::sync() することで他のクライアントからその関数の情報を見ることができるようになります。

hidden属性をつけると他のMemberやWebUIから関数の存在を隠すことができます。
Client::funcs()でその関数の存在を確認したりFunc::args()などでの情報の取得ができなくなります。
名前がわかっていれば他Memberからでも実行は可能です。

* C++では func.hidden(true) で設定できます。
* ![js ver1.0.4](https://img.shields.io/badge/1.0.4~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
JavaScript, Pythonではset()関数(Pythonデコレータの場合はfunc())の引数に指定することで設定できます。

### 実行条件 (C++)

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

## FuncListener (関数の待ち受け)
![c++ ver1.3](https://img.shields.io/badge/1.3~-00599c?logo=C%2B%2B)

呼び出されたとき実行する関数を登録する代わりに、呼び出されたかどうかを監視し任意のタイミングで値を返すということもできます。

```cpp
wcli.funcListener("hoge").listen();
```
で待ち受けを開始し、func.set()と同様関数が登録され他クライアントから見られるようになります。
(listen()ではブロックはしません)

引数を受け取りたい場合は
```cpp
wcli.funcListener("hoge").listen(3);
```
のように引数の個数を指定するか、または
```cpp
wcli.funcListener("hoge").setArgs({...}).listen();
```
とすると通常のfuncと同様引数のオプションを設定可能です。
(func.setArgs()はfunc.set()の後でしたが、funcListenerではlisten()の前に実行する必要があります。)

その後、任意のタイミングで
```cpp
std::optional<webcface::FuncCallHandle> handle = wcli.funcListener("hoge").fetchCall();
```
とすることで関数が呼び出されたかどうかを調べることができます。
listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetchCallで取得する前に呼び出し元に例外が投げられます(呼び出されていないのと同じことになります)

その関数がまだ呼び出されていない場合はstd::nulloptが返ります。
関数が呼び出された場合、`handle.args()`で引数を調べ、`handle.respond()`で関数のreturnと同様に関数の終了を示したり値を返してください。
また`handle.reject()`でエラーメッセージを返すことができます(呼び出し元にはruntime_errorを投げたものとして返ります)

## 関数の実行

Func::run() で関数を実行できます。引数を渡すこともでき、戻り値もそのまま返ってきます。
他クライアントの関数も同様にrun()で実行させることができます。

実行した関数が例外を投げた場合、また引数の個数が一致しない場合などはrun()が例外を投げます。
```cpp
double ans = wcli.member("a").func("hoge").run(1, "aa");
```
(C++, Python) Funcオブジェクトに()と引数をつけて直接呼び出すことでも同様に実行できます。
(C++の`operator()`, Pythonの`__call__`)

Func::runAsync() は関数の実行を開始し、終了を待たずに続行します。
戻り値として AsyncFuncResult クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。
```cpp
AsyncFuncResult res = wcli.member("a").func("hoge").runAsync(1, "aa");
double ans = res.result.get();
```
JavaScriptではrun()はなく、runAsync()のみ使えます。

AsyncFuncResultからは started と result が取得できます。
started は対象の関数が存在して実行が開始したときにtrueになり、存在しなければ即座にfalseとなります。
どちらも返ってこない場合は通信に失敗しています。

result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。

* C++ではstartedとresultはstd::shared_futureです。取得できるまで待機するならget(), ブロックせず完了したか確認したければwait_for()などが使えます。例外はresult.get()が投げます。
* Pythonではstartedとresultは取得できるまで待機するgetterです。例外の場合はresultの取得時に投げます。また、取得可能になったかどうかをstarted_readyとresult_readyで取得できます。
* JavaScriptではstartedとresultはPromiseです。awaitで待機したり、then()とcatch()でコールバックを設定できます。


## AnonymousFunc

名前を指定しないFuncです。
[View](./13_view.md)のボタンなどのコールバックに関数を設定したい場合に使うことができます。

wcli.func() で取得できるようにする予定だが未実装。(現在はViewComponentの内部でのみ使われている)

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [View](13_view.md) | [Log](40_log.md) |

</div>
