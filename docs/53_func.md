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

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::func からFuncオブジェクトを作り、 Func::set() で関数を登録し、Client::sync()することで送信されます

    関数は関数オブジェクト(ラムダ式など)でもokです。
    引数、戻り値は整数、実数、bool、文字列型であれば自由に指定できます。  
    <span class="since-c">3.0</span> 1次元のリスト (std::vector, std::array) も引数と戻り値に使用可能です。
    ```cpp
    void hoge() {
        std::cout << "hello, world!" << std::endl;
    }
    double fuga(int a, std::string_view b) {
        // ver2.9以前は const std::string & b
        return 3.1415;
    }
    wcli.func("hoge").set(hoge);
    wcli.func("fuga").set(fuga);

    // or, using lambda:
    wcli.func("hoge").set([](){ /* ... */ });
    wcli.func("fuga").set([](int a, const std::string &b){ return 3.1415; });
    ```
    * <del>set() の代わりに代入演算子(Func::operator=)でも同様のことができます</del>
        * <span class="since-c">2.0</span> 代入演算子はdeprecatedになりました
    * 同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
    ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。
    * 関数の中で例外をthrowした場合WebCFace側でcatchされ、エラーメッセージが呼び出し元に返ります。
    * <span class="since-c">2.0</span>
    set()で登録した関数はClient::sync()時に同じスレッドで実行されます。
    そのため長時間かかる関数を登録するとその間他の処理がブロックされることになります。
        * 関数を非同期(別スレッド)で実行したい場合は Func::setAsync() を使用してください。
        ver1.11以前ではset()で登録した関数はすべて非同期実行されていたので、こちらが従来のset()と同じ挙動になります。
        * ただしその場合排他制御が必要なら登録する関数内で適切に行ってください
    ```cpp
    wcli.func("hoge").setAsync([](){
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return "hello";
    });
    ```

    \note
    * <span class="since-c">2.10</span>
    WebCFaceが文字列データを管理する際 std::string を用いないようになったため、
    std::string で文字列を受け取ると(const参照であっても)コピーが発生します。
    std::string_view または webcface::StringView を用いると効率的です
        * ただしver2.9以前では std::string_view 引数の関数をsetできないため、互換性を維持する必要があるなら const std::string & にしましょう

    <!--
    <span class="since-c">2.0</span>
    戻り値にstd::futureまたはstd::shared_futureを返す関数も登録可能です。
    その場合戻り値のfutureの結果を待機する(get() を呼び出す)のは別スレッドで行われます。
    ```cpp
    wcli.func("hoge").set([](){
        // ここはsync()のスレッドで同期実行
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
    set()で登録した関数はClient::sync() の中で呼び出されるため、
    * sync() が呼び出される頻度が少ない場合、呼び出されてから実際に関数を実行するまでにラグが生じます。
    その場合はsync()を呼び出す頻度を上げるか、loopSync() を使ってください ([4-1. Client](./41_client.md) 参照)
    * set()で登録した関数内で他のクライアントの関数をrun()やrunAsync()で呼び出して結果を受け取ろうとするとデッドロックしてしまいます。
    (結果を受け取る処理も sync() で行われるため)
    ```cpp
    wcli.func("hoge").set([&](){
        wcli.member("foo").func("piyo").runAsync().waitFinish(); // deadlock
        
        std::async([&]{
            wcli.member("foo").func("piyo").waitFinish(); // ok (非同期実行)
        });

        // またはそもそもこの関数をsetAsync()で登録すればok
    });
    ```

    <span></span>

- <b class="tab-title">JavaScript</b>
    Client.func からFuncオブジェクトを作り、 Func.set() で関数を登録し、Client.sync()することで送信されます

    引数、戻り値はnumber, bool, string型であればいくつでも自由に指定できます。  
    <span class="since-js">1.12</span> 1次元の配列も引数に使用可能です。
    ```ts
    wcli.func("hoge").set(() => {/* ... */});
    wcli.func("hoge").set((a: number, b: string) => 3.1415);
    ```
    * 関数の中で例外をthrowした場合WebCFace側でcatchされ、エラーメッセージが呼び出し元に返ります。
    * 同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
    ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。

- <b class="tab-title">Python</b>
    Client.func からFuncオブジェクトを作り、 Func.set() で関数を登録し、Client.sync()することで送信されます
    ```py
    def hoge(a, b):
        return 3.1415

    wcli.func("hoge").set(hoge)
    wcli.func("lambda").set(lambda x: return x + 5) # ラムダ式なども可
    ```
    * 関数の中で例外をraiseした場合WebCFace側でcatchされ、エラーメッセージが呼び出し元に返ります。
    * 同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
    ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。
    * setを明示的に呼び出す代わりにfuncオブジェクトをデコレータにすると簡単に登録できます。
    ```py
    @wcli.func("hoge")
    def hoge(a, b):
        return 3.1415
    ```
    * デコレータとして使用する場合func()の引数に関数名を書くのを省略すると実際の関数の名前が自動で取得され設定されます。
        * `()` は必須です。(`@wcli.func` をデコレータとして使わないでください。)
    ```py
    @wcli.func()
    def hoge(a, b):
        return 3.1415
    ```
    * <span class="since-py">2.0</span>
    set() で登録した関数はClient.sync()時に同じスレッドで実行されます。
    そのため長時間かかる関数を登録するとその間他の処理がブロックされることになります。
        * 関数を非同期(別スレッド)で実行したい場合は Func.set_async() を使用してください。
        ver1.11以前ではset()で登録した関数はすべて非同期実行されていたので、こちらが従来のset()と同じ挙動になります。
        * set_async() で登録した関数は新しいスレッド(threading.Thread)で実行されます。
    ```py
    def hoge(a, b):
        time.sleep(5)

    wcli.func("hoge").set_async(hoge)
    ```

    \warning
    <span class="since-py">2.0</span>
    set()で登録した関数はClient.sync() の中で呼び出されるため、
    * sync() が呼び出される頻度が少ない場合、呼び出されてから実際に関数を実行するまでにラグが生じます。
    その場合はsync()を呼び出す頻度を上げるか、sync(timeout) を使ってください ([4-1. Client](./41_client.md) 参照)
    * set()で登録した関数内で他のクライアントの関数をrun()やrun_async()で呼び出して結果を受け取ろうとするとデッドロックしてしまいます。
    (結果を受け取る処理も sync() で行われるため)
    ```python
    @wcli.func()
    def hoge():
        wcli.member("foo").func("piyo").run() # deadlock
        
        def wait_piyo():
            wcli.member("foo").func("piyo").run() # ok (非同期実行)

        threading.Thread(target=wait_piyo)

        # またはそもそもこの関数をsetAsync()で登録すればok
    ```

    <span></span>

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

### CallHandle

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">2.4</span>

    引数に webcface::CallHandle をとる関数をセットすることもできます。
    ```cpp
    wcli.func("hoge").set([](webcface::CallHandle handle){
        double arg = handle.args().at(0).asDouble();
        handle.respond(123); // return value
    }).setArgs({
        webcface::Arg("a").type(webcface::ValType::double_),
    });
    ```
    * 通常の関数のset()の場合と同様、関数を非同期(別スレッド)で実行したい場合は setAsync() を使用してください。
    * 関数のsetのあとに、 setArgs() で受け取りたい引数の個数分の webcface::Arg() を渡し、引数名や戻り値の型を指定します。
        * 引数名などの情報が不要な場合でも引数の個数分 Arg() を渡す必要があります。
    * callbackが呼び出されたとき、引数に渡されたhandleを介して `handle.args()` でFuncを呼び出した引数を取得できます。
        * setArgs() で指定した引数の個数と呼び出し時の個数が一致しない場合は、callbackが実行される前に呼び出し元にエラーメッセージが投げられます。
        (そのため登録した関数の側で引数の個数チェックをする必要はないです)
    * 値を返すのはreturnではなくhandleを介して `handle.respond(戻り値)` を使います。
        * 戻り値が不要な場合はrespondに何も渡さず `handle.respond();` とします。
    * エラーメッセージを返すには `handle.reject(エラーメッセージ)` を使います。
        * 関数の中で例外をthrowした場合もエラーメッセージが呼び出し元に返ります。
    * respondかrejectを呼び出すまで、Funcの呼び出し元には関数呼び出しの結果は送られません。
        * そのためhandleを関数の外のスコープの変数に保存(コピー)したり、別スレッドに持ち込むことで任意のタイミングで結果を返すことも可能です。
    
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
    * set時には受け取りたい引数の型、個数、戻り値の型を指定します。
    型は WCF_VAL_NONE, WCF_VAL_STRING, WCF_VAL_BOOL, WCF_VAL_INT, WCF_VAL_DOUBLE が指定できます。
    * 同じ名前のFuncに複数回関数をセットすると上書きされ、後に登録した関数のみが呼び出されます。
    ただし引数や戻り値の型などの情報は更新されず、最初の関数のものと同じになります。
    * <span class="since-c">2.0</span>
    wcfFuncSet(), wcfFuncSetW() で登録した関数はwcfSync()時に同じスレッドで実行されます。
    そのため長時間かかる関数を登録するとその間他の処理がブロックされることになります。
        * 関数を非同期(別スレッド)で実行したい場合は wcfFuncSetAsync(), wcfFuncSetAsyncW() を使用してください。
        ver1.11以前ではwcfFuncSet()で登録した関数はすべて非同期実行されていたので、こちらが従来のwcfFuncSet()と同じ挙動になります。
        * (ただしその場合排他制御が必要なら登録する関数内で適切に行ってください)
    ```cpp
    wcfFuncSetAsync(wcli, "hoge", args_type, 3, WCF_VAL_INT, callback, &user_data);
    ```
    * callbackが呼び出されたとき、引数に渡されたhandleを介して `handle->args` でFuncを呼び出した引数を取得できます。
        * set時に指定した引数の個数と呼び出し時の個数が一致しない場合は、callbackが実行される前に呼び出し元にエラーメッセージが投げられます。
        (そのため引数の個数チェックをする必要はないです)
    * 値を返すのはreturnではなくhandleを介して wcfFuncRespond, (<span class="since-c">2.0</span> wcfFuncRespondW) を使います。
    ```c
    wcfMultiVal ans = wcfValD(123.45);
    wcfFuncRespond(handle, &ans);
    ```
    * 戻り値が不要な場合は `wcfFuncRespond(handle, NULL);` とします。
    * エラーメッセージを返すには wcfFuncReject, (<span class="since-c">2.0</span> wcfFuncRejectW) を使います。
    (`wcfFuncReject(handle, NULL);` も可)
    ```c
    wcfFuncReject(handle, "エラーメッセージ");
    ```
    * <del>respondもrejectもせずにreturnした場合は自動的に空の値でrespondします。</del>
        * <span class="since-c">2.0</span>
        respondかrejectを呼び出すまで、Funcの呼び出し元には関数呼び出しの結果は送られません。
        * そのためhandleポインタを関数の外のスコープの変数に保存したり、別スレッドに持ち込むことで任意のタイミングで結果を返すことも可能です。
    
    \warning
    <span class="since-c">2.0</span>
    wcfFuncSet()で登録した関数はwcfSync() の中で呼び出されるため、
    * wcfSync() が呼び出される頻度が少ない場合、呼び出されてから実際に関数を実行するまでにラグが生じます。
    その場合はwcfSync()を呼び出す頻度を上げるか、wcfLoopSync() を使ってください ([4-1. Client](./41_client.md) 参照)
    * wcfFuncSet()で登録した関数内で他のクライアントの関数をwcfFuncRun()やwcfFuncRunAsync()で呼び出して結果を受け取ろうとするとデッドロックしてしまいます。
    (結果を受け取る処理も wcfSync() で行われるため)
    その場合はwcfFuncSetAsync()を使用してください。

    <span></span>

- <b class="tab-title">Python</b>
    \since <span class="since-py">2.2</span>

    引数に [webcface.CallHandle](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.CallHandle) をとる関数をセットすることもできます。
    ```py
    def hoge(handle):
        a = handle.args[0]
        handle.respond(123) # return value

    wcli.func("hoge").set(hoge,
        handle=True,
        return_type=int,
        args=[Arg("a", type=int)]
    )
    ```
    * 通常の関数との区別のため、set()の引数に `handle=True` を渡すか、または引数の型アノテーションで `CallHandle` を指定するかのどちらかが必要です。
        * デコレータで登録する場合はset()の代わりに func() の引数に`handle=True`を渡してください。
    * 通常の関数のset()の場合と同様、関数を非同期(別スレッド)で実行したい場合は set_async() を使用してください。
    * 関数のset時に、戻り値の型と、受け取りたい引数の個数分の [webcface.Arg](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.Arg) を渡し、引数名や型を指定してください。
        * デコレータで登録する場合はset()の代わりに func() の引数に渡してください。
    * callbackが呼び出されたとき、引数に渡されたhandleを介して `handle.args` でFuncを呼び出した引数を取得できます。
        * set時に指定した引数の個数と呼び出し時の個数が一致しない場合は、callbackが実行される前に呼び出し元にエラーメッセージが投げられます。
        (そのため登録した関数の側で引数の個数チェックをする必要はないです)
    * 値を返すのはreturnではなくhandleを介して `handle.respond(戻り値)` を使います。
        * 戻り値が不要な場合はrespondに何も渡さず `handle.respond()` とします。
    * エラーメッセージを返すには `handle.reject(エラーメッセージ)` を使います。
        * 関数の中で例外をthrowした場合もエラーメッセージが呼び出し元に返ります。
    * respondかrejectを呼び出すまで、Funcの呼び出し元には関数呼び出しの結果は送られません。
        * そのためhandleを関数の外のスコープの変数に保存(コピー)したり、別スレッドに持ち込むことで任意のタイミングで結果を返すことも可能です。

</div>

### 引数と戻り値型

<div class="tabbed">

- <b class="tab-title">C++</b>
    引数型や戻り値型は関数の型から自動で取得されますが、
    引数名などの情報や引数に設定する条件などを Func::setArgs() でセットすることができます。
    設定可能な情報の一覧は webcface::Arg を参照

    関数をsetする前に呼ぶとエラーになります。
    また、実際の関数の引数と個数が一致していなければ std::invalid_argument を投げます。
    ```cpp
    wcli.func("fuga").setArgs({
        webcface::Arg("a").init(100),
        webcface::Arg("b").option({"aaa", "bbb", "ccc"}),
    });
    ```

- <b class="tab-title">JavaScript</b>
    引数名、引数や戻り値の型、その他の引数に関する情報をset関数の引数で指定します。
    引数のオプションに関しては [Arg](https://na-trium-144.github.io/webcface-js/interfaces/Arg.html) を参照
    ```ts
    wcli.func("hoge").set(
        () => { /*...*/ }, // 関数
        valType.float_, // 戻り値の型
        [ // 引数の情報
            { name: "a", type: valType.int_, init: 100 },
            { name: "b", type: valType.string_, option: ["aaa", "bbb", "ccc"] },
        ],
    );
    ```

- <b class="tab-title">Python</b>
    Pythonではset()関数の引数にオプションで args と return_type を渡すことで型やその他の情報を指定できます。
    argsの引数に関しては [webcface.Arg](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.Arg) を参照

    引数名については実際の引数名が自動的に取得されます。
    ```py
    from webcface import Arg

    def hoge(a, b):
        return 3.1415

    wcli.func("hoge").set(hoge, return_type=float, args=[
        Arg(type=int, init=100),
        Arg(type=str, option=["aaa", "bbb", "ccc"]),
    ])
    ```
    デコレータで登録する場合はset()の代わりにfunc()の引数に渡してください。
    ```py
    @wcli.func(return_type=float, args=[
        Arg(type=int, init=100),
        Arg(type=str, option=["aaa", "bbb", "ccc"]),
    ])
    def hoge(a, b):
        return 3.1415
    ```
    引数と戻り値の型アノテーションをすると、型の指定を省略できます。
    また、デフォルト引数も指定されていればそれが使われます。
    ```py
    def hoge(a: int = 100, b: str = "aaa") -> float:
        return 3.1415

    wcli.func("hoge").set(hoge, args=[
        Arg(),
        Arg(option=["aaa", "bbb", "ccc"]),
    ])
    ```

    <span class="since-py">2.2</span>
    実際の関数の引数とargsで指定した個数が一致していなければAssertionErrorになります。

</div>


#### 型変換

* 関数登録時に指定した型と呼び出したときに渡した引数の型が違う場合、呼び出された側のライブラリが自動的に変換してから関数に渡します。
* (ver1.5.3〜1.9.0のserverではすべて文字列型に置き換えられてしまうバグあり、ver1.9.1で修正)
* 変換規則は基本的には呼び出された側の言語仕様の標準に従います。
    * 文字列→数値は10進数で変換されます。小数点以下の桁数や、指数表記にするかどうかは未規定です
    * 数値→文字列も10進数としてパースされます。数値でない文字列が渡された場合の処理は未規定です
* bool→文字列
    * C++: 0, 1
    * Python: <del>False, True</del>
    <span class="since-py">3.0</span> 0, 1
    * JavaScript: false, true
* 文字列→bool
    * C++: <del>`"1"` のみtrue</del>
    <span class="since-c">1.9.1</span> 空文字列でないときtrue
    * Python: 空文字列でないときTrue
    * JavaScript: 空文字列でないときtrue
* <span class="since-c">3.0</span> <span class="since-js">1.12</span>
1次元の配列も使用可能です。型が一致していない場合配列でない単一の値は要素数1の配列と相互変換されます。
* (C++) 引数を webcface::ValAdaptor, <span class="since-c">3.0</span> webcface::ValAdaptorVector 型にすると型変換を行わずに値を受け取ることができます。
* (C) set時に指定した引数の型によらず、callHandleからは常にint,double,文字列のいずれでも値を受け取ることができます。
* (JavaScript) set時に引数の型の情報を指定しなかった場合、型変換を行わず送られてきた値をそのまま関数に渡します。
* (Python) <span class="since-py">3.0</span>
関数登録時に指定した型が int,float,bool,str のいずれでもないもしくは未指定の場合、
型変換を行わず送られてきた値をそのまま関数に渡します。

### 関数をWebUIから隠す

(serverが<span class="since-c">1.10</span>以降の場合)

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

### FuncListener

呼び出されたとき実行する関数を登録する代わりに、呼び出されたかどうかを監視し任意のタイミングで値を返すということもできます。
特にCのFFIで関数ポインタが利用できない場合に代わりに使えます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">1.5</span>

    ```cpp
    wcli.funcListener("hoge").listen();
    ```
    で待ち受けを開始し、func.set()と同様関数が登録され他クライアントから見られるようになります。
    (listen()自体は関数呼び出しを待機することはありません)

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
    * その関数がまだ呼び出されていない場合はstd::nulloptが返ります。
    * 関数が呼び出された場合、 CallHandle::args() で呼び出された引数を取得できます。
        * 各引数は <del>ValAdaptor</del> <span class="since-c">3.0</span> ValAdaptorVector 型で取得でき、
        <del>`asStringRef()`</del>, `asString()`, `asBool()`, <del>`as<double>()`</del>,
        <span class="since-c">2.0</span> <del>`asWStringRef()`</del>, `asWString()`, `asDouble()`, `asInt()`, `asLLong()`,
        <span class="since-c">2.10</span> `asStringView()`, `asWStringView()`, `asVector<T>()`, `asArray<T, N>()`
        で型を指定して取得できます。
        * (std::string, double, bool などの型にキャストすることでも値を得られます。)
        * listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetchCallで取得する前に呼び出し元に例外が投げられます
        (そのため引数の個数チェックをする必要はないです)
    * 通常の関数のreturnの代わりに、`handle.respond()` で関数呼び出しの終了を表し、戻り値を返すことができます。
    * またthrowの代わりに `handle.reject()` でエラーメッセージを返すことができます。

    詳細は webcface::CallHandle
    (ver2.0で FuncCallHandle から CallHandle に名前変更)

- <b class="tab-title">C</b>
    ```c
    wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli, "hoge", args_type, 3, WCF_VAL_INT);
    ```
    で待ち受けを開始し、wcfFuncSet() と同様関数が登録され他クライアントから見られるようになります。
    (wcfFuncListen()自体は関数呼び出しを待機することはありません)

    その後、任意のタイミングで
    ```c
    wcfFuncCallHandle *handle;
    wcfFuncFetchCall(wcli, "hoge", &handle);
    ```
    とすることで関数が呼び出されたかどうかを調べることができます。
    * その関数がまだ呼び出されていない場合は`WCF_NOT_CALLED`が返ります。
    * 関数が呼び出された場合`WCF_OK`が返り、`handle->args`に引数が格納されます。
        * listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetchCallで取得する前に呼び出し元に例外が投げられます
        (そのため引数の個数チェックをする必要はないです)
    * 通常の関数のreturnの代わりに、 wcfFuncRespond, (<span class="since-c">2.0</span> wcfFuncRespondW) で関数呼び出しの終了を表し、戻り値を返すことができます。
    ```c
    wcfMultiVal ans = wcfValD(123.45);
    wcfFuncRespond(handle, &ans);
    ```
    * <span class="since-c">1.9</span> 戻り値が不要な場合は `wcfFuncRespond(handle, NULL);` とします。
    * または wcfFuncReject, (<span class="since-c">2.0</span> wcfFuncRejectW) でエラーメッセージを返すことができます。
    (`wcfFuncReject(handle, NULL);` も可)
    ```c
    wcfFuncReject(handle, "エラーメッセージ");
    ```

- <b class="tab-title">Python</b>
    \since <span class="since-py">2.2</span>

    ```py
    wcli.funcListener("hoge").listen()
    ```
    で待ち受けを開始し、func.set()と同様関数が登録され他クライアントから見られるようになります。
    (listen()自体は関数呼び出しを待機することはありません)

    引数を受け取りたい場合は
    ```py
    wcli.funcListener("hoge").listen(args=[...])
    ```
    のように引数の個数分の Arg を指定してください。
    
    その後、任意のタイミングで
    ```py
    handle = wcli.funcListener("hoge").fetch_call()
    ```
    とすることで関数が呼び出されたかどうかを調べることができます。
    * その関数がまだ呼び出されていない場合はNoneが返ります。
    * 関数が呼び出された場合、 handle.args で呼び出された引数を取得できます。
        * listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetch_callで取得する前に呼び出し元に例外が投げられます
        (そのため引数の個数チェックをする必要はないです)
    * 通常の関数のreturnの代わりに、`handle.respond()` で関数呼び出しの終了を表し、戻り値を返すことができます。
    * またthrowの代わりに `handle.reject()` でエラーメッセージを返すことができます。

    詳細は [webcface.CallHandle](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.CallHandle) 

</div>

### Funcの表示順 (index)

WebUI(ver1.13〜)でFunc一覧を表示する際、名前順の表示とFuncが登録された順の表示を切り替えることができます。

Funcが登録された順番(index)は送信側のクライアントライブラリで自動的に管理されますが、
手動で上書きして表示順を指定することも可能です。

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">2.8</span>

    ```cpp
    wcli.func("hoge").set(...).setIndex(1);
    wcli.func("fuga").set(...).setIndex(3);
    wcli.func("piyo").set(...).setIndex(2);
    ```
    のように、関数のsetのあとにindexを指定できます。

- <b class="tab-title">JavaScript</b>
    \since <span class="since-js">1.11</span>

    set関数の4番目の引数で指定できます。
    ```ts
    wcli.func("hoge").set(
        () => { /*...*/ }, // 関数
        valType.float_, // 戻り値の型
        [...], // 引数の情報
        1, // index
    );
    ```

</div>

## 関数の情報の取得

関数の引数や戻り値、
(<span class="since-c">2.8</span><span class="since-js">1.11</span>関数が登録された順番)
の情報を取得できます。
他のデータ型と違ってデータをリクエストする機能はなく、
関数の情報はクライアントが関数を登録してsync()した時点で送られてきます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::func() でFuncクラスのオブジェクトが得られ、
    Func::returnType() や Func::args() で関数の引数や戻り値の情報を取得できます。
    ```cpp
    std::vector<webcface::Arg> args = wcli.member("foo").func("hoge").args();
    webcface::ValType return_type = wcli.member("foo").func("hoge").returnType();
    ```
    引数の情報については webcface::Arg を参照
    また、戻り値型は webcface::ValType というenum型で得られます。

    <span class="since-c">2.8</span>
    関数がset()などで登録された順番 (もしくは送信側クライアントが上書きした値) を Func::index() で取得できます。

- <b class="tab-title">JavaScript</b>
    Member.func() でFuncクラスのオブジェクトが得られ、
    Func.returnType や Func.args で関数の引数や戻り値の情報を取得できます。
    ```ts
    const args = wcli.member("foo").func("hoge").args;
    const returnType = wcli.member("foo").func("hoge").returnType;
    ```
    引数の情報については [Arg](https://na-trium-144.github.io/webcface-js/interfaces/Arg.html) を参照

    <span class="since-js">1.11</span>
    関数がset()などで登録された順番 (もしくは送信側クライアントが上書きした値) を Func.index で取得できます。

- <b class="tab-title">Python</b>
    Member.func() でFuncクラスのオブジェクトが得られ、
    Func.return_type や Func.args で関数の引数や戻り値の情報を取得できます。
    ```python
    args = wcli.member("foo").func("hoge").args
    return_type = wcli.member("foo").func("hoge").return_type
    ```
    引数の情報については [webcface.Arg](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.Arg) を参照

</div>

### Entry

他のデータ型と同様、関数が存在するかどうかを取得することができます。
詳細は [4-3. Field](./43_field.md) を参照してください

## 関数の実行

他クライアントに登録された関数を呼び出すことができます。
(自分でsetした関数を自分で実行することも一応可能です)

引数を渡したり、戻り値またはエラーメッセージを取得することができます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Func::runAsync() で関数を呼び出し、完了を待たずに続行します。
    戻り値として <del>AsyncFuncResult</del> <span class="since-c">2.0</span> Promise クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。

    <span class="since-c">2.0</span>
    Promiseでは以下のメソッドが使用可能です。
    * reached(): 関数呼び出しのメッセージが相手のクライアントに到達したらtrue、それまでの間はfalseです。
        * waitReach(), waitReachFor(), waitReachUntil(): reached()がtrueになるまで待機します。
        For, Until の場合はタイムアウトを指定します。
    * found(): reached()がtrueになった後、相手のクライアントが関数の実行を開始したらtrue、指定したクライアントまたは関数が存在しなかった場合falseです。
        * reached()がfalseの間はfalseです。
        * runAsync呼び出し時にクライアントがサーバーに接続していなかった場合は、関数呼び出しメッセージを送信することなく即座にfalseになります
    * finished(): 関数の実行が完了し戻り値かエラーメッセージを受け取ったらtrue、それまでの間はfalseです。
        * waitFinish(), waitFinishFor(), waitFinishUntil(): finished()がtrueになるまで待機します。
        For, Until の場合はタイムアウトを指定します。
    * response(): 関数の戻り値です。
    webcface::ValAdaptorVector 型で返り、
    <del>`asStringRef()`</del>, `asString()`, <del>`asWStringRef()`</del>, `asWString()`, `asBool()`, `asDouble()`, `asInt()`, `asLLong()`,
    <span class="since-c">2.10</span> `asStringView()`, `asWStringView()`, `asVector<T>()`, `asArray<T, N>()`
    またはstatic_castにより型変換できます。
    * rejection(), rejectionW(): 関数が例外を返した場合そのエラーメッセージを表す文字列です。
    またその場合 isError() がtrueになります。

    ```cpp
    Promise res = wcli.member("foo").func("hoge").runAsync(1, "aa");
    res.waitReach();
    if(res.found()){
        // 関数hogeが存在し、実行が開始された
        res.waitFinish();
        if(res.isError()){
            // res.rejection() がエラーメッセージ
        }else{
            // res.response() が戻り値
        }
    }else{
        // 関数hogeが存在しないか未接続で呼び出し失敗
    }
    ```

    \warning
    <span class="since-c">2.0</span>
    上の例のようにwaitReach(), waitFinish() などで結果が返ってくるまで待機することができますが、
    これらが結果を受信するためには Client::sync() が必要なため、別スレッドでsync()が呼ばれていなければデッドロックします。

    <span></span>

    * `onReach()`, `onFinish()` で値が返ってきたときに実行されるコールバックを設定することができます。
        * 引数にはそのPromise自身が渡されますが、(キャプチャするなどして)必要なければ引数なしの関数も設定可能です
        * コールバックは Client::sync() の中から呼び出されます。
    ```cpp
    Promise res = wcli.member("foo").func("hoge").runAsync(1, "aa");
    res.onReach([](Promise res){
        if(res.found()){
            // 関数hogeが存在し、実行が開始された
        }else{
            // 関数hogeが存在しないか未接続で呼び出し失敗
        }
    });
    res.onFinish([](Promise res){
        if(res.isError()){
            // res.rejection() がエラーメッセージ
        }else{
            // res.response() が戻り値
        }
    });
    ```

    \note
    onReach, onFinish を設定した時点ですでに関数の実行が完了していた場合は、そのときにコールバックが呼ばれます。
    したがってコールバックはどの状況で設定したとしても必ず1回呼ばれます。
    (呼ばれたあとにコールバックを再設定したりしても2度目が呼ばれることはありません)

    <span></span>

- <b class="tab-title">C</b>
    wcfFuncRunAsync, (<span class="since-c">2.0</span> wcfFuncRunAsyncW)
    で関数を呼び出し、完了を待たずに続行します。
    受け取った wcfPromise を使って、
    wcfFuncGetResult, (<span class="since-c">2.0</span> wcfFuncGetResultW)
    後から関数の戻り値や例外を取得できます。
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
    * 完了していれば、wcfFuncGetResultは`WCF_OK`を返し、結果が wcfMultiVal 型で取得できます。
        * このとき wcfPromise オブジェクトは破棄され使えなくなります (再度 wcfFuncGetResult を呼ぶことはできません)
        * <span class="since-c">1.7</span>
        また、受け取った wcfMultiVal オブジェクトは、不要になったら wcfDestroy で破棄してください。  
    * <span class="since-c">2.0</span>
    戻り値を取得する必要がない場合は、 wcfDestroy でwcfPromiseも破棄することができます。

    wcfFuncWaitResult, (<span class="since-c">2.0</span> wcfFuncWaitW)  は関数の実行が完了し結果が返ってくるまで待機します。
    
    \warning
    <span class="since-c">2.0</span>
    wcfFuncWaitResult() が結果を受信するためには wcfSync() が必要なため、別スレッドでwcfSync()が呼ばれていなければデッドロックします。

    <span></span>

- <b class="tab-title">JavaScript</b>
    \note
    <span class="since-js">1.8</span> C++の Promise 型に合わせて名前変更しました。
    以前の名前もまだ使えます。

    Func.runAsync() で関数を呼び出すと、戻り値として <del>AsyncFuncResult</del> FuncPromise
    クラスのオブジェクトが返り、後から関数の戻り値や例外を取得できます。

    <del>AsyncFuncResult</del> FuncPromise からは
    <del>started</del> reach と
    <del>result</del> finish
    が取得できます。
    いずれもPromise型で、awaitで待機したり、then()とcatch()でコールバックを設定できます。
    * reach (started) は対象の関数が存在して実行が開始したときにtrueになり、指定したクライアントまたは関数が存在しなかった場合falseとなります。
        * <span class="since-js">1.8</span>
        runAsync呼び出し時にクライアントがサーバーに接続していなかった場合は、関数呼び出しメッセージを送信することなく即座にfalseが返ります
    * finish (result) は実行が完了したときに返ります。関数の戻り値を返すか、または発生した例外のメッセージを含むErrorでrejectします。
        * <span class="since-js">1.8</span> Rejectする値は常にError型になっています。
        (1.7以前は関数がthrowしたオブジェクトをそのまま返していた)

    ```ts
    import { FuncPromise } from "webcface";
    const res: FuncPromise = wcli.member("foo").func("hoge").runAsync(1, "aa");
    res.reach.then((found: boolean) => {
        if (found) {
            // 関数hogeが存在し、実行が開始された
        } else {
            // 関数hogeが存在しないか未接続で呼び出し失敗
        }
    });
    res.finish.then((ret: number | boolean | string) => {
        // ret が戻り値
    }).catch((e) => {
        // (e as Error).message がエラーメッセージ
    });
    ```

- <b class="tab-title">Python</b>
    Func.run_async() で関数を呼び出すと、
    戻り値として <del>AsyncFuncResult</del> <span class="since-py">2.0</span> Promise クラスのオブジェクトが返り、
    後から関数の戻り値や例外を取得できます。

    <span class="since-py">2.0</span>
    Promiseでは以下のメソッドが使用可能です。
    * reached: 関数呼び出しのメッセージが相手のクライアントに到達したらTrue、それまでの間はFalseです。
        * wait_reach(): reached がTrueになるまで待機します。
        timeoutを指定することもできます。
    * found: reachedがTrueになった後、相手のクライアントが関数の実行を開始したらTrue、指定したクライアントまたは関数が存在しなかった場合Falseです。
        * reached がFalseの間はFalseです。
        * run_async呼び出し時にクライアントがサーバーに接続していなかった場合は、関数呼び出しメッセージを送信することなく即座にFalseになります
    * finished: 関数の実行が完了し戻り値かエラーメッセージを受け取ったらTrue、それまでの間はFalseです。
        * wait_finish(): finished()がTrueになるまで待機します。
        timeoutを指定することもできます。
    * response: 関数の戻り値です。
    int, float, bool, str型のいずれかで返ります
    * rejection: 関数が例外を返した場合そのエラーメッセージを表す文字列です。
    またその場合 is_error がTrueになります。

    ```python
    res = wcli.member("foo").func("hoge").run_async(1, "aa")
    res.wait_reach()
    if res.found:
        # 関数hogeが存在し、実行が開始された
        res.wait_finish()
        if res.is_error:
            # res.rejection がエラーメッセージ
        else:
            # res.response が戻り値
    else:
         関数hogeが存在しないか未接続で呼び出し失敗
    ```

    \warning
    <span class="since-py">2.0</span>
    上の例のようにwait_reach(), wait_finish() などで結果が返ってくるまで待機することができますが、
    これらが結果を受信するためには Client.sync() が必要なため、別スレッドでsync()が呼ばれていなければデッドロックします。

    <span></span>

    * `on_reach()`, `on_finish()` で値が返ってきたときに実行されるコールバックを設定することができます。
        * 引数にはそのPromise自身が渡されます
        * eventのコールバックと同様、デコレータとして使ってコールバックを設定することもできます。
        * コールバックは Client.sync() の中から呼び出されます。
    ```python
    res = wcli.member("foo").func("hoge").run_async(1, "aa")

    @res.on_reach
    def on_reach(res: Promise):
        if res.found:
            # 関数hogeが存在し、実行が開始された
        else:
            # 関数hogeが存在しないか未接続で呼び出し失敗

    @res.on_finish
    def on_finish(res: Promise):
        if res.is_error:
            # res.rejection がエラーメッセージ
        else:
            # res.response が戻り値
    ```
    \note
    on_reach, on_finish を設定した時点ですでに関数の実行が完了していた場合は、そのときにコールバックが呼ばれます。
    したがってコールバックはどの状況で設定したとしても必ず1回呼ばれます。
    (呼ばれたあとにコールバックを再設定したりしても2度目が呼ばれることはありません)

    <span></span>

    Func.run() は関数を呼び出し、結果が返ってくるまで待機します。
    例外が返ってきた場合はRuntimeErrorを、また呼び出しに失敗した場合は webcface.FuncNotFoundError をraiseします。
    ```python
    result = wcli.member("foo").func("hoge").run(1, "aa")
    ```

    * wait_reach(), wait_finish() と同様 Client.sync() が呼ばれていないとデッドロックするので注意してください。
    
    Funcオブジェクトに()と引数をつけて直接呼び出すことでも同様に実行できます。
    (`Func.__call__`)

</div>

<details><summary>AsyncFuncResult.started と result (ver2.0からdeprecated)</summary>

<div class="tabbed">

- <b class="tab-title">C++</b>

    AsyncFuncStarted::started と AsyncFuncResult::result はstd::shared_futureであり、取得できるまで待機するならget(), ブロックせず完了したか確認したければwait_for()などが使えます。
    * started は対象の関数が存在して実行が開始したときにtrueになり、指定したクライアントまたは関数が存在しなかった場合falseとなります。
        * <span class="since-c">2.0</span>
        runAsync呼び出し時にクライアントがサーバーに接続していなかった場合は、関数呼び出しメッセージを送信することなく即座にfalseになります
    * result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。
        * 実行した関数が例外を返した場合はresult.get()がstd::runtime_errorを投げます。
    * <span class="since-c">2.0</span>
    started.get() や result.get() はver2.0以降デッドロックする可能性があります。
    詳細は上に書かれている waitReach(), waitFinish() の注意を参照してください

- <b class="tab-title">Python</b>
    AsyncFuncResultからは started と result が取得できます。
    * started は対象の関数が存在して実行が開始したときにtrueになり、指定したクライアントまたは関数が存在しなかった場合falseとなります。
    * result は実行が完了したときに返ります。関数の戻り値、または発生した例外の情報を含んでいます。

    startedとresultは取得できるまで待機するgetterです。例外の場合はresultの取得時に投げます。
    また、取得可能になったかどうかをstarted_readyとresult_readyで取得できます。
    詳細は [webcface.AsyncFuncResult](https://na-trium-144.github.io/webcface-python/webcface.func_info.html#webcface.func_info.AsyncFuncResult) を参照

</div>

</details>

<details><summary>CallbackListを返す AsyncFuncResult::onStarted(), onResult() (C++ ver1.11)</summary>

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

    戻り値は webcface::ValAdaptorVector 型で返ります。
    整数、実数、bool、stringにキャストできます。  
    <span class="since-c">1.10</span> また、明示的にキャストするなら <del>`asStringRef()`(const参照)</del>, `asString()`, `asBool()`, <del>`as<整数or実数型>()`</del> も使えます。  
    <span class="since-c">2.0</span> <del>`asWStringRef()`</del>, `asWString()`, `asDouble()`, `asInt()`, `asLLong()`,
    <span class="since-c">2.10</span> `asStringView()`, `asWStringView()`, `asVector<T>()`, `asArray<T, N>()`
    も使えます。

    \warning
    start()を呼んで通信を開始する前にrun()を呼び出してしまうとデッドロックします。

    <span></span>

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

    <span></span>

</div>

</details>

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [5-2. Text](52_text.md) | [5-4. View](54_view.md) |

</div>
