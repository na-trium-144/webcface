# Func

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::Func
* JavaScript [Func](https://na-trium-144.github.io/webcface-js/classes/Func.html)
* Python [webcface.Func](https://na-trium-144.github.io/webcface-python/webcface.func.html#webcface.func.Func)

他Memberから呼び出せる関数の登録、
また他Memberの関数の呼び出しができます。

## 関数の登録

Client::func からFuncオブジェクトを作り、 Func::set() で関数を登録し、Client::sync()することで送信されます

<div class="tabbed">

- <b class="tab-title">C++</b>
    関数はstd::functionに変換できるものであればなんでもokです。
    引数、戻り値はint, double, bool, std::string型であればいくつでも自由に指定できます。
    ```cpp
    wcli.func("hoge").set([](){ /* ... */ });
    wcli.func("fuga").set([](int a, const std::string &b){ return 3.1415; });
    ```
    set() の代わりに代入演算子(Value::operator=)でも同様のことができます

- <b class="tab-title">JavaScript</b>
    引数、戻り値はnumber, bool, string型であればいくつでも自由に指定できます。
    ```ts
    wcli.func("hoge").set(() => {/* ... */});
    wcli.func("hoge").set((a: number, b: string) => 3.1415);
    ```

- <b class="tab-title">Python</b>
    引数と戻り値のアノテーションをすると自動で取得されます。
    ```py
    def hoge(a: int, b: str) -> float:
        return 3.1415

    wcli.func("hoge").set(hoge)
    wcli.func("lambda").set(lambda x: return x + 5) # ラムダ式なども可
    ```
    
    setを明示的に呼び出す代わりにfuncオブジェクトをデコレータにすると簡単に登録できます。
    この場合func()の引数に関数名を書くのを省略すると実際の関数の名前が自動で取得され設定されます。
    (明示的に関数名を渡しても良い)
    ```py
    @wcli.func()
    def hoge(a: int, b: str) -> float:
        return 3.1415
    ```

</div>

\note
同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。

### 引数と戻り値型

<div class="tabbed">

- <b class="tab-title">C++</b>
    Func::setArgs() で引数名などの情報や、引数に設定する条件をセットすることができます。
    設定可能な情報の一覧は webcface::Arg を参照

    関数をセットする前に呼ぶとエラーになります。
    また、実際の関数の引数と個数が一致していなければエラーになります。
    ```cpp
    wcli.func("fuga").setArgs({
        webcface::Arg("a").init(100),
        webcface::Arg("b").option({"aaa", "bbb", "ccc"}),
    });
    ```

- <b class="tab-title">JavaScript</b>
    set関数の引数で指定します。
    型情報を取得できないJavaScript, TypeScriptではここで型情報も指定する必要があります。
    引数のオプションに関しては [Arg](https://na-trium-144.github.io/webcface-js/interfaces/Arg.html) を参照
    ```ts
    wcli.func("hoge").set(hoge, valType.float_, [
        { name: "a", type: valType.int_, init: 100 },
        { name: "b", type: valType.string_, option: ["aaa", "bbb", "ccc"] },
    ]);
    ```

- <b class="tab-title">Python</b>
    Pythonではset()関数の引数(デコレータとして使う場合はfunc()の引数)にオプションで args と return_type を渡すことで指定できます。
    argsの引数に関しては [webcface.Arg](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.Arg) を参照

    関数に型アノテーションがない場合はここで引数と戻り値の型も指定する必要があります。  
    引数名は自動で取得されるので不要ですが、明示的に指定することもできます。
    ```py
    wcli.func("hoge").set(hoge, return_type=float, args=[
        Arg(type=int, init=100),
        Arg(type=str, option=["aaa", "bbb", "ccc"]),
    ])
    ```

</div>

### hidden属性

関数の設定後 Client::sync() することで他のクライアントからその関数の情報を見ることができるようになります。

hidden属性をつけると他のMemberやWebUIから関数の存在を隠すことができます。
Client::funcEntries()でその関数の存在を確認したりFunc::args()などでの情報の取得ができなくなります。
ただし関数の名前がわかっていれば他Memberからでも実行は可能です。

<div class="tabbed">

- <b class="tab-title">C++</b>
    C++では
    ```cpp
    func.hidden(true)
    ```
    で設定できます。

- <b class="tab-title">JavaScript</b>
    <span class="since-js">1.0.4</span>
    set()関数の4番目の引数にtrueを指定することでできます。
    ```ts
    wcli.func("hoge").set(hoge, valType.float_, [ ... ], true);
    ```

- <b class="tab-title">Python</b>
    set()関数(デコレータの場合はfunc())の引数に指定することで設定できます。
    ```py
    wcli.func("hoge").set(hoge, return_type=float, args=[...], hidden=True)
    ```

</div>

### 実行条件

<div class="tabbed">

- <b class="tab-title">C++</b>
    C++では呼び出された関数は別スレッドで非同期に実行されます。
    これをメインスレッドと同期させたい場合は実行条件を設定することができます。
    ```cpp
    wcli.func("fuga").setRunCondOnSync();
    ```
    とすると呼び出された関数の実行は wcli.sync() のときに行われます。
    ```cpp
    struct ScopeGuard {
        static std::mutex m;
        ScopeGuard() { m.lock(); }
        ~ScopeGuard() { m.unlock(); }
    };
    wcli.func("fuga").setRunCondScopeGuard<ScopeGuard>();
    ```
    とすると任意のScopeGuardクラスを使うことができます
    (実行前にScopeGuardのコンストラクタ、実行後にデストラクタが呼ばれます)

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

</div>

## FuncListener

呼び出されたとき実行する関数を登録する代わりに、呼び出されたかどうかを監視し任意のタイミングで値を返すということもできます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <span class="since-c">1.5</span>
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

    詳細は webcface::FuncCallHandle

- <b class="tab-title">C</b>
    ```c
    wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli, "hoge", args_type, 3, WCF_VAL_INT);
    ```
    で待ち受けを開始し、func.set()と同様関数が登録され他クライアントから見られるようになります。
    (wcfFuncListen()ではブロックはしません)

    その後、任意のタイミングで
    ```c
    wcfFuncCallHandle *handle;
    wcfFuncFetchCall(wcli, "hoge", &handle);
    ```
    とすることで関数が呼び出されたかどうかを調べることができます。
    listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetchCallで取得する前に呼び出し元に例外が投げられます(呼び出されていないのと同じことになります)

    その関数がまだ呼び出されていない場合は`WCF_NOT_CALLED`が返ります。
    関数が呼び出された場合`WCF_OK`が返り、`handle->args`に引数が格納されます。
    ```c
    wcfMultiVal ans = wcfValD(123.45);
    wcfFuncRespond(handle, &ans);
    ```
    で関数のreturnと同様に関数を終了して値を返したり、
    ```c
    wcfFuncReject(handle, "エラーメッセージ");
    ```
    でエラーメッセージを返すことができます(呼び出し元にはruntime_errorを投げたものとして返ります)

</div>

## 関数の情報の取得
Member::func() でFuncクラスのオブジェクトが得られ、
Func::returnType() や Func::args() で関数の引数や戻り値の情報を取得できます。

他のデータ型と違ってデータをリクエストする機能はなく、
関数の情報はクライアントが関数を登録してsync()した時点で送られてきます。

### Entry

~~Member::funcs() で~~ そのMemberが送信している関数のリストが得られます  
<span class="since-c">1.6</span>
<span class="since-py">1.1</span>
Member::funcEntries() に変更

また、Member::onFuncEntry() で新しく関数が追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

## 関数の実行

Func::run() で関数を実行できます。引数を渡すこともでき、戻り値もそのまま返ってきます。
他クライアントの関数も同様にrun()で実行させることができます。

実行した関数が例外を投げた場合、また引数の個数が一致しない場合などはrun()が例外を投げます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    double ans = wcli.member("foo").func("hoge").run(1, "aa");
    ```
    Funcオブジェクトに()と引数をつけて直接呼び出すことでも同様に実行できます。
    (`Func::operator()`)

- <b class="tab-title">C</b>
    ```c
    wcfMultiVal args[3] = {
        wcfValI(42), // int
        wcfValD(1.5), // double
        wcfValS("aaa"), // string
    };
    wcfMultiVal *ans;
    int ret = wcfFuncRun(wcli_, "a", "b", args, 3, &ans);
    // ex.) ret = WCF_OK, ans->as_double = 123.45
    ```
    関数が存在しない場合`WCF_NOT_FOUND`を返します。
    関数が例外を投げた場合`WCF_EXCEPTION`を返し、ret->as_strにエラーメッセージが入ります。

- <b class="tab-title">Python</b>
    ```py
    ans = wcli.member("foo").func("hoge").run(1, "aa")
    ```
    Funcオブジェクトに()と引数をつけて直接呼び出すことでも同様に実行できます。
    (`Func::__call__`)

</div>

### runAsync

Func::runAsync() は関数の実行を開始し、終了を待たずに続行します。
戻り値として AsyncFuncResult クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。

AsyncFuncResultからは started と result が取得できます。
* started は対象の関数が存在して実行が開始したときにtrueになり、存在しなければ即座にfalseとなります。
    どちらも返ってこない場合は通信に失敗しています。
* result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    AsyncFuncResult res = wcli.member("foo").func("hoge").runAsync(1, "aa");
    double ans = res.result.get();
    ```
    startedとresultはstd::shared_futureです。取得できるまで待機するならget(), ブロックせず完了したか確認したければwait_for()などが使えます。例外はresult.get()が投げます。

    詳細は webcface::AsyncFuncResult を参照してください。

- <b class="tab-title">C</b>
    ```c
    wcfMultiVal args[3] = {
        wcfValI(42),
        wcfValD(1.5),
        wcfValS("aaa"),
    };
    wcfAsyncFuncResult *async_res;
    wcfFuncRunAsync(wcli_, "a", "b", args, 3, &async_res);

    wcfMultiVal *ans;
    int ret = wcfFuncGetResult(async_res, &ans);
    // ex.) ret = WCF_OK, ans->as_double = 123.45

    int ret = wcfFuncWaitResult(async_res, &ans);
    // ex.) ret = WCF_OK, ans->as_double = 123.45
    ```
    関数の実行がまだ完了していなければwcfFuncGetResultは`WCF_NOT_RETURNED`を返します。

    wcfFuncWaitResult は関数の実行が完了し結果が返ってくるまで待機します。

- <b class="tab-title">JavaScript</b>
    \note
    JavaScriptではrun()はなく、runAsync()のみ使えます。

    ```ts
    import { AsyncFuncResult } from "webcface";
    const res: AsyncFuncResult = wcli.member("foo").func("hoge").runAsync(1, "aa");
    res.result.then((ret: number | boolean | string) => {
        // ...
    }).catch((e) => {
        // ...
    });
    ```
    startedとresultはPromiseです。awaitで待機したり、then()とcatch()でコールバックを設定できます。
    詳細は [AsyncFuncResult](https://na-trium-144.github.io/webcface-js/classes/AsyncFuncResult.html) を参照してください

- <b class="tab-title">Python</b>
    ```python
    res = wcli.member("foo").func("hoge").runAsync(1, "aa")
    ans = res.result
    ```
    startedとresultは取得できるまで待機するgetterです。例外の場合はresultの取得時に投げます。
    また、取得可能になったかどうかをstarted_readyとresult_readyで取得できます。
    詳細は [webcface.AsyncFuncResult](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.AsyncFuncResult) を参照

</div>

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [RobotModel](21_robot_model.md) | [Log](40_log.md) |

</div>
