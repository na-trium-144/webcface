# 5-3. Func

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::Func (`webcface/func.h`)
* C Reference: c_wcf/func.h
* JavaScript [Func](https://na-trium-144.github.io/webcface-js/classes/Func.html)
* Python [webcface.Func](https://na-trium-144.github.io/webcface-python/webcface.func.html#webcface.func.Func)

他Memberから呼び出せる関数の登録、
また他Memberの関数の呼び出しができます。

## 関数の登録

Client::func からFuncオブジェクトを作り、 Func::set() で関数を登録し、Client::sync()することで送信されます

\note
同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。

<div class="tabbed">

- <b class="tab-title">C++</b>
    関数はstd::functionに変換できるものであればなんでもokです。
    引数、戻り値は整数、実数、bool、文字列型であればいくつでも自由に指定できます。
    ```cpp
    wcli.func("hoge").set([](){ /* ... */ });
    wcli.func("fuga").set([](int a, const std::string &b){ return 3.1415; });
    ```
    ~~set() の代わりに代入演算子(Value::operator=)でも同様のことができます~~

    <span class="since-c">2.0</span>
    set()で登録した関数はsync()時に同じスレッドで実行されます。
    そのため長時間かかる関数を登録するとその間他の処理がブロックされることになります。  
    関数を非同期(別スレッド)で実行したい場合は Func::setAsync() を使用してください。
    ver1.11以前ではset()で登録した関数はすべて非同期実行されていたので、こちらが従来のset()と同じ挙動になります。
    (ただしその場合排他制御が必要なら登録する関数内で適切に行ってください)
    ```cpp
    wcli.func("hoge").setAsync([](){
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return "hello";
    });
    ```
    <!--
    <span class="since-c">2.0</span>
    戻り値にstd::futureまたはstd::shared_futureを返す関数も登録可能です。
    その場合戻り値のfutureの結果を待機する(get() を呼び出す)のは別スレッドで行われます。
    ```cpp
    wcli.func("hoge").set([](){
        // ここはrecv()のスレッドで同期実行
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return std::async(std::launch::async, []{
            // ここは別スレッドで実行
            // ちなみにこの場合 std::launch::deferred でもよい
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return "hello";
        });
    });
    ```
    -->

    \warning
    <span class="since-c">2.0</span>
    WebCFaceで他のクライアントの関数をrun()で呼び出して結果を受け取るには受信処理が必要になるので、
    set()で登録した関数内でrun()を呼ぶとデッドロックしてしまいます。
    ```cpp
    wcli.func("hoge").set([&](){
        return wcli.member("foo").func("piyo").run(); // deadlock
        return std::async([&]{
            wcli.member("foo").func("piyo").run(); // ok (非同期実行)
        });
        // またはそもそもこの関数をsetAsync()で登録すればok
    });
    ```

- <b class="tab-title">C</b>
    \since <span class="since-c">1.9</span>

    wcfFuncSet, (<span class="since-c">2.0</span> wcfFuncSetW) で関数ポインタを登録できます。  
    登録する関数の引数は wcfFuncCallHandle*, (<span class="since-c">2.0</span> wcfFuncCallHandleW*) と void* の2つで、
    前者は引数のデータを取得したり結果を返すのに使用します。  
    後者には登録時に任意のデータのポインタを渡すことができます。(使用しない場合はNULLでよいです。)
    ```c
    void callback(wcfFuncCallHandle *handle, void *user_data_p) {
        struct UserData *user_data = (struct UserData *)user_data_p;
        // do something
        wcfMultiVal ans = wcfValI(123); // return value
        wcfFuncRespond(handle, ans);
    }
    wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    struct UserData user_data = {...};
    wcfFuncSet(wcli, "hoge", args_type, 3, WCF_VAL_INT, callback, &user_data);
    ```
    set時には受け取りたい引数の型、個数、戻り値の型を指定します。
    型は WCF_VAL_NONE, WCF_VAL_STRING, WCF_VAL_BOOL, WCF_VAL_INT, WCF_VAL_DOUBLE が指定できます。

    callbackが呼び出されたとき `handle->args` に引数が格納されます。
    set時に指定した引数の個数と呼び出し時の個数が一致しない場合、callbackが実行される前に呼び出し元に例外が投げられます(呼び出されていないのと同じことになります)

    値を返すのはreturnではなくhandleを介して行います。
    ```c
    wcfMultiVal ans = wcfValD(123.45);
    wcfFuncRespond(handle, &ans);
    ```
    で関数のreturnと同様に関数を終了して値を返します
    (戻り値が不要な場合は `wcfFuncRespond(handle, NULL);` も可)
    ```c
    wcfFuncReject(handle, "エラーメッセージ");
    ```
    でエラーメッセージを返すことができます(呼び出し元にはruntime_errorを投げたものとして返ります)
    (`wcfFuncReject(handle, NULL);` も可)

    respondもrejectもせずにreturnした場合は自動的に空の値でrespondします。

    <span class="since-c">2.0</span>
    wcfFuncSet(), wcfFuncSetW() で登録した関数はwcfSync()時に同じスレッドで実行されます。
    そのため長時間かかる関数を登録するとその間他の処理がブロックされることになります。  
    関数を非同期(別スレッド)で実行したい場合は wcfFuncSetAsync(), wcfFuncSetAsyncW() を使用してください。
    ver1.11以前ではwcfFuncSet()で登録した関数はすべて非同期実行されていたので、こちらが従来のwcfFuncSet()と同じ挙動になります。
    (ただしその場合排他制御が必要なら登録する関数内で適切に行ってください)
    ```cpp
    wcfFuncSetAsync(wcli, "hoge", args_type, 3, WCF_VAL_INT, callback, &user_data);
    ```
    \warning
    <span class="since-c">2.0</span>
    WebCFaceで他のクライアントの関数をwcfFuncRun()で呼び出して結果を受け取るには受信処理が必要になるので、
    wcfFuncSet()で登録した関数内でwcfFuncRun()を呼ぶとデッドロックしてしまいます。
    その場合はwcfFuncSetAsync()を使用してください。


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

<details><summary>setした関数にWebCFace側で排他制御をかける機能(〜ver1.11まで)</summary>

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

</details>

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

### 関数をWebUIから隠す

\since <span class="since-c">1.10</span>

関数の名前を半角ピリオドから始めると、Entryが他クライアントに送信されなくなり、
他のMemberやWebUIから関数の存在を隠すことができます。
(Valueなど他のデータ型についても同様です。)

なお半角ピリオド2つから始まる名前はwebcface内部の処理で利用する場合があるので使用しないでください。

以前のバージョンでは名前に関係なく関数を非表示にするオプションがありましたが、削除しました。

<details><summary>以前のバージョン</summary>
hidden属性をつけると他のMemberやWebUIから関数の存在を隠すことができます。
Client::funcEntries()でその関数の存在を確認したりFunc::args()などでの情報の取得ができなくなります。
ただし関数の名前がわかっていれば他Memberからでも実行は可能です。

<div class="tabbed">

- <b class="tab-title">C++</b>
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
</details>

## FuncListener

呼び出されたとき実行する関数を登録する代わりに、呼び出されたかどうかを監視し任意のタイミングで値を返すということもできます。
特にCのFFIで関数ポインタが利用できない場合に代わりに使えます。

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
    std::optional<webcface::CallHandle> handle = wcli.funcListener("hoge").fetchCall();
    ```
    とすることで関数が呼び出されたかどうかを調べることができます。
    listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetchCallで取得する前に呼び出し元に例外が投げられます(呼び出されていないのと同じことになります)

    その関数がまだ呼び出されていない場合はstd::nulloptが返ります。
    関数が呼び出された場合、`handle.args()`で引数を調べ、`handle.respond()`で関数のreturnと同様に関数の終了を示したり値を返してください。
    また`handle.reject()`でエラーメッセージを返すことができます(呼び出し元にはruntime_errorを投げたものとして返ります)

    詳細は webcface::CallHandle
    (ver2.0で FuncCallHandle から CallHandle に名前変更)

- <b class="tab-title">C</b>
    ```c
    wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli, "hoge", args_type, 3, WCF_VAL_INT);
    ```
    で待ち受けを開始し、wcfFuncSet() と同様関数が登録され他クライアントから見られるようになります。
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
    で関数のreturnと同様に関数を終了して値を返します
    (<span class="since-c">1.9</span> 戻り値が不要な場合は `wcfFuncRespond(handle, NULL);` も可)
    ```c
    wcfFuncReject(handle, "エラーメッセージ");
    ```
    でエラーメッセージを返すことができます(呼び出し元にはruntime_errorを投げたものとして返ります)
    (`wcfFuncReject(handle, NULL);` も可)

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

他クライアントにsetされた関数を呼び出すことができます。
(自分でsetした関数を自分で実行することも一応可能です)

引数を渡したり、戻り値またはエラーメッセージを取得することができます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Func::runAsync() で関数を呼び出し、完了を待たずに続行します。
    戻り値として <del>AsyncFuncResult</del> <span class="since-c">2.0</span> Promise クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。

    \deprecated
    <del>AsyncFuncStarted::started と AsyncFuncResult::result はstd::shared_futureであり、取得できるまで待機するならget(), ブロックせず完了したか確認したければwait_for()などが使えます。</del>
    * started は対象の関数が存在して実行が開始したときにtrueになり、指定したクライアントまたは関数が存在しなかった場合falseとなります。
        * <span class="since-c">2.0</span>
        runAsync呼び出し時にクライアントがサーバーに接続していなかった場合は、関数呼び出しメッセージを送信することなく即座にfalseになります
    * result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。
        * 実行した関数が例外を返した場合はresult.get()がstd::runtime_errorを投げます。
    * <span class="since-c">2.0</span>
    Promise::started, Promise::result として今までどおり使用可能ですがdeprecatedです。
        * started.get() や result.get() はver2.0以降デッドロックする可能性があります。
        詳細は後述の waitReach(), waitFinish() を参照してください
    
    <span class="since-c">2.0</span>
    Promiseでは以下のメソッドが使用可能です。
    * reached(): 関数呼び出しのメッセージが相手のクライアントに到達したらtrue、それまでの間はfalseです。
        * waitReach(), waitReachFor(), waitReachUntil(): reach()がtrueになるまで待機します。
        For, Until の場合はタイムアウトを指定します。
    * found(): reached()がtrueになった後、相手のクライアントが関数の実行を開始したらtrue、指定したクライアントまたは関数が存在しなかった場合falseです。
        * reach()がfalseの間はfalseです。
        * runAsync呼び出し時にクライアントがサーバーに接続していなかった場合は、関数呼び出しメッセージを送信することなく即座にfalseになります
    * finished(): 関数の実行が完了し戻り値かエラーメッセージを受け取ったらtrue、それまでの間はfalseです。
        * waitFinish(), waitFinishFor(), waitFinishUntil(): finished()がtrueになるまで待機します。
        For, Until の場合はタイムアウトを指定します。
    * response(): 関数の戻り値です。
    webcface::ValAdaptor 型で返り、
    `asStringRef()`, `asString()`, `asWStringRef()`, `asWString()`, `asBool()`, `asDouble()`, `asInt()`, `asLLong()`
    またはstatic_castにより型変換できます。
    * rejection(), rejectionW(): 関数が例外を返した場合そのエラーメッセージを表す文字列です。
    またその場合 isError() がtrueになります。

    \warning
    <span class="since-c">2.0</span>
    waitReach(), waitFinish() などで結果が返ってくるまで待機することができますが、
    これらが結果を受信するためには Client::sync() が必要なため、別スレッドでsync()が呼ばれていなければデッドロックします。

    * `onReach()`, `onFinish()` で値が返ってきたときに実行されるコールバックを設定することができます。
        * 引数にはそのPromise自身が渡されますが、(キャプチャするなどして)必要なければ引数なしの関数も設定可能です
    ```cpp
    Promise res = wcli.member("foo").func("hoge").runAsync(1, "aa");
    res.onReach([](Promise res){
        std::cout << "func hoge() " << res.found() ? "started" : "not started" << std::endl;
    });
    res.onFinish([](Promise res){
        if(!res.isError()){
            double ans = res.response();
        }else{
            std::cout << "Error: " << res.rejection() << std::endl;
        }
    });
    ```

    \note
    onReach, onFinish を設定した時点ですでに関数の実行が完了していた場合は、そのときにコールバックが呼ばれます。
    したがってコールバックはどの状況で設定したとしても必ず1回呼ばれます。
    (呼ばれたあとにコールバックを再設定したりしても2度目が呼ばれることはありません)

- <b class="tab-title">C</b>
    ```c
    wcfMultiVal args[3] = {
        wcfValI(42),
        wcfValD(1.5),
        wcfValS("aaa"),
    };
    wcfPromise *async_res;
    wcfFuncRunAsync(wcli_, "a", "b", args, 3, &async_res);

    wcfMultiVal *ans;
    int ret = wcfFuncGetResult(async_res, &ans);
    // int ret = wcfFuncWaitResult(async_res, &ans);
    // ex.) ret = WCF_OK, ans->as_double = 123.45

    wcfDestroy(ans);
    ```
    * (ver1.11まで wcfAsyncFuncResult 型でしたが ver2.0で wcfPromise に名前変更しました)
    * 関数の実行がまだ完了していなければwcfFuncGetResultは`WCF_NOT_RETURNED`を返します。
    * wcfFuncWaitResult は関数の実行が完了し結果が返ってくるまで待機します。
    
    \warning
    <span class="since-c">2.0</span>
    wcfFuncWaitResult() が結果を受信するためには wcfSync() が必要なため、別スレッドでwcfSync()が呼ばれていなければデッドロックします。

    * <span class="since-c">1.7</span>
    結果が格納されているポインタは、不要になったら wcfDestroy(ans); で破棄してください。  
        * wcfPromiseについては結果をansに渡した時点で破棄され使えなくなるためwcfDestroyの呼び出しは不要です。
    * <span class="since-c">2.0</span>
    戻り値を取得する必要がない場合は、 wcfDestroy(async_res); でwcfPromiseも破棄することができます。

- <b class="tab-title">JavaScript</b>
    Func.runAsync() で関数を呼び出すと、戻り値として AsyncFuncResult クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。

    AsyncFuncResultからは started と result が取得できます。
    * started は対象の関数が存在して実行が開始したときにtrueになり、指定したクライアントまたは関数が存在しなかった場合falseとなります。
    * result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。

    startedとresultはPromiseです。awaitで待機したり、then()とcatch()でコールバックを設定できます。
    詳細は [AsyncFuncResult](https://na-trium-144.github.io/webcface-js/classes/AsyncFuncResult.html) を参照してください

    ```ts
    import { AsyncFuncResult } from "webcface";
    const res: AsyncFuncResult = wcli.member("foo").func("hoge").runAsync(1, "aa");
    res.result.then((ret: number | boolean | string) => {
        // ...
    }).catch((e) => {
        // ...
    });
    ```

- <b class="tab-title">Python</b>
    Func.run() は関数を呼び出し、結果が返ってくるまで待機します。
    ```py
    ans = wcli.member("foo").func("hoge").run(1, "aa")
    ```
    Funcオブジェクトに()と引数をつけて直接呼び出すことでも同様に実行できます。
    (`Func::__call__`)

    Func.run_async() で関数を呼び出すと、戻り値として AsyncFuncResult クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。

    AsyncFuncResultからは started と result が取得できます。
    * started は対象の関数が存在して実行が開始したときにtrueになり、指定したクライアントまたは関数が存在しなかった場合falseとなります。
    * result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。

    startedとresultは取得できるまで待機するgetterです。例外の場合はresultの取得時に投げます。
    また、取得可能になったかどうかをstarted_readyとresult_readyで取得できます。
    詳細は [webcface.AsyncFuncResult](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.AsyncFuncResult) を参照

    ```python
    res = wcli.member("foo").func("hoge").run_async(1, "aa")
    ans = res.result
    ```

</div>

<details><summary>CallbackListを返す AsyncFuncResult::onStarted(), onResult() (ver1.11)</summary>

<span class="since-c">1.11</span>
`onStarted()`, `onResult()` で値が返ってきたときに実行されるイベントが取得でき、コールバックを設定することができます。
([eventpp::CallbackList 型](https://github.com/wqking/eventpp/blob/master/doc/callbacklist.md)の参照で返ります)  
```cpp
AsyncFuncResult res = wcli.member("foo").func("hoge").runAsync(1, "aa");
res.onStarted().append([](bool started){
    std::cout << "func hoge() " << started ? "started" : "not started" << std::endl;
});
res.onResult().append([](std::shared_future<webcface::ValAdaptor> result){
    try{
        double ans = result.get();
    }catch(const std::exception &e){
        std::cout << e.what() << std::endl;
    }
});
```

</details>

<details><summary>C++の Func::run(), Cの wcfFuncRun() (ver2.0からdeprecated)</summary>

<div class="tabbed">

- <b class="tab-title">C++</b>
    Func::run() で関数を実行できます。引数を渡すこともでき、戻り値もそのまま返ってきます。
    他クライアントの関数も同様にrun()で実行させることができます。

    実行した関数が例外を投げた場合、また引数の個数が一致しない場合などはrun()が例外を投げます。

    対象のクライアントと通信できない場合、また指定した関数が存在しない場合は webcface::FuncNotFoundError を投げます。

    ```cpp
    double ans = wcli.member("foo").func("hoge").run(1, "aa");
    ```
    Funcオブジェクトに()と引数をつけて直接呼び出すことでも同様に実行できます。
    (`Func::operator()`)

    戻り値は webcface::ValAdaptor 型で返ります。
    整数、実数、bool、stringにキャストできます。  
    <span class="since-c">1.10</span> また、明示的にキャストするなら `asStringRef()`(const参照), `asString()`, `asBool()`, <del>`as<整数or実数型>()`</del> も使えます。  
    <span class="since-c">2.0</span> `asWStringRef()`, `asWString()`, `asDouble()`, `asInt()`, `asLLong()` も使えます。

    \warning
    start()を呼んで通信を開始する前にrun()を呼び出してしまうとデッドロックします。

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

    wcfDestroy(ans);
    ```
    関数が存在しない場合`WCF_NOT_FOUND`を返します。
    関数が例外を投げた場合`WCF_EXCEPTION`を返し、ret->as_strにエラーメッセージが入ります。

    <span class="since-c">1.7</span>
    結果が格納されているポインタは、不要になったら wcfDestroy(ans); で破棄してください。

    \warning
    wcfStart()を呼んで通信を開始する前にwcfFuncRun()を呼び出してしまうとデッドロックします。

</div>

</details>

### 型変換

* 引数の型が違う場合、関数登録時に指定した型に自動的に変換されてから呼び出されます。
* (ver1.5.3〜1.9.0のserverではすべて文字列型に置き換えられてしまうバグあり、ver1.9.1で修正)
* 変換は受信側のライブラリで行われ、基本的にその言語仕様に従って変換します
* c++ではstring→boolの変換は文字列が"1"のみtrueだったが <span class="since-c">1.9.1</span> 空文字列でないときtrueに変更

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [RobotModel](21_robot_model.md) | [Log](40_log.md) |

</div>
