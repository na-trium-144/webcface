# Value

\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::Value
* JavaScript [Value](https://na-trium-144.github.io/webcface-js/classes/Value.html)
* Python [webcface.Value](https://na-trium-144.github.io/webcface-python/webcface.value.html#webcface.value.Value)

数値データ、または1次元数値配列を送受信する型です。

## コマンドライン

```sh
webcface-send 
```
を実行し、数字を入力すると送信されます。(1つ入力するごとに改行してください)

オプションでclientやデータの名前を変更できます。詳細は `webcface-send -h` を参照

## 送信

Client::value からValueオブジェクトを作り、 Value::set() でデータを代入し、Client::sync()することで送信されます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.value("hoge").set(5);
    wcli.value("fuga").set({1, 2, 3, 4, 5});
    ```
     (C++のみ) set() の代わりに代入演算子(Value::operator=)でも同様のことができます。
    また、 operator+= など、doubleやintの変数で使える各種演算子も使えます
    ```cpp
    wcli.value("hoge") = 5;
    ```

- <b class="tab-title">JavaScript</b>
    ```ts
    wcli.value("hoge").set(5);
    wcli.value("fuga").set([1, 2, 3, 4, 5]);
    ```

- <b class="tab-title">Python</b>
    ```python
    wcli.value("hoge").set(5)
    wcli.value("fuga").set([1, 2, 3, 4, 5])
    ```

</div>

### 複数の値をまとめて送る

ROSのTopicではPointやTransformなど目的に応じてさまざまな型が用意されていますが、
WebCFaceではそういう場合はValueを複数用意して送信することを想定しています。

\todo
Pythonの辞書型への対応は未実装

<div class="tabbed">

- <b class="tab-title">C++</b>
    webcface::Value::Dict オブジェクトを使うと複数の値をまとめて送ることができます。
    これは構造体などのデータを送るときに使えます
    ```cpp
    struct A {
        double x, y;
        operator webcface::Value::Dict() const {
            return {
                {"x", x},
                {"y", y},
                {"vec", {1, 2, 3}}, // vectorも入れられます
                {"a", {             // 入れ子にもできます
                    {"a", 1},
                    {"b", 1},
                }}
            }
        }
    };

    A a_instance;
    wcli.value("a").set(a_instance); // Dictにキャストされる

    /* 結果は以下のようになる
      value("a.x") -> a_instance.x
      value("a.y") -> a_instance.y
      value("a.vec") -> {1, 2, 3}
      value("a.a.a") -> 1
      value("a.a.b") -> 1
    */
    ```

- <b class="tab-title">JavaScript</b>
    オブジェクトを渡すことができます。
    ```ts
    wcli.value("a").set({
        x: 1,
        y: 2,
        vec: [1, 2, 3],
        a: {
            a: 1,
            b: 1,
        },
    });
    /* 結果は以下のようになる
      value("a.x") -> 1
      value("a.y") -> 2
      value("a.vec") -> {1, 2, 3}
      value("a.a.a") -> 1
      value("a.a.b") -> 1
    */
    ```

</div>

## 受信

WebCFaceのクライアントは初期状態ではデータを受信しません。
リクエストを送って初めてサーバーからデータが順次送られてくるようになります。
これはValueに限らず、これ以降説明する他のデータ型のfieldについても同様です。

![pub-sub](https://github.com/na-trium-144/webcface/raw/main/docs/images/pub-sub.png)

Member::value() でValueクラスのオブジェクトが得られ、
Value::tryGet(), Value::tryGetVec(), Value::tryGetRecurse() で値のリクエストをするとともに受信した値を取得できます。
それぞれ 1つのdoubleの値、vector<double>、Dict を返します。
(Dict はC++のみ)

例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    std::optional<double> hoge = wcli.member("foo").value("hoge").tryGet();
    ```
    初回の呼び出しではまだ受信していないため、
    tryGet(), tryGetVec(), tryGetRecurse() はstd::nulloptを返します。  
    get(), getVec(), getRecurse() はstd::nulloptの代わりにデフォルト値を返します。  
    また、doubleやstd::vector<double>, Value::Dict などの型にキャストすることでも同様に値が得られます。
- <b class="tab-title">JavaScript</b>
    ```ts
    const hoge: double | null = wcli.member("foo").value("hoge").tryGet();
    ```
    初回の呼び出しではまだ受信していないため、
    tryGet(), tryGetVec() はnullを返します。  
    get(), getVec() はnullの代わりにデフォルト値を返します。
- <b class="tab-title">Python</b>
    ```python
    hoge = wcli.member("foo").value("hoge").try_get()
    ```
    初回の呼び出しではまだ受信していないため、
    try_get(), try_get_vec() はNoneを返します。  
    get(), getVec() はNoneの代わりにデフォルト値を返します。

</div>

~~その後Client::sync()したときに実際にリクエストが送信され、~~  
<span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
自動的に別スレッドでリクエストが送信され、それ以降は値が得られるようになります。
そのため、次の例のように繰り返し取得して使ってください。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    while(true) {
        std::optional<double> val = wcli.member("a").value("hoge").tryGet();
        if(val) {
            std::cout << "hoge = " << *val << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ```

- <b class="tab-title">JavaScript</b>
    ```ts
    setInterval(() => {
        const val = wcli.member("a").value("hoge").tryGet();
        if(val !== null){
            console.log(`hoge = ${val}`);
        }
    }, 100);
    ```
    
- <b class="tab-title">Python</b>
    ```python
    while True:
        val = wcli.member("foo").value("hoge").try_get()
        if val is not None:
            print(f"hoge = {val}")
        time.sleep(0.1)
    ```

</div>


\note
<span class="since-js">1.1</span>
<span class="since-py"></span>
Value::request()で明示的にリクエストを送信することもできます。

### 時刻

Value::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

\note
Pythonでは Member.sync_time() です
C++,JavaScriptでも今後仕様変更して統一するかも

### Entry

~~Member::values() で~~ そのMemberが送信しているvalueのリストが得られます  
<span class="since-c">1.6</span>
Member::valueEntries() に変更

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    for(const webcface::Value &v: wcli.member("foo").valueEntries()){
        // ...
    }
    ```
- <b class="tab-title">JavaScript</b>
    ```js
    for(const v of wcli.member("foo").values()){
        // ...
    }
    ```
- <b class="tab-title">Python</b>
    ```python
    for v in wcli.member("foo").values():
        # ...
    ```

</div>

Member::onValueEntry() で新しくデータが追加されたときのコールバックを設定できます

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.member("foo").onValueEntry().appendListener([](webcface::Value v){ /* ... */ });
    ```
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Value } from "webcface";
    wcli.member("foo").onValueEntry.on((v: Value) => { /* ... */ });
    ```
- <b class="tab-title">Python</b>
    ```python
    def value_entry(v: webcface.Value):
        pass
    wcli.member("foo").on_value_entry.connect(value_entry)
    ```

</div>

ただし、コールバックを設定する前から存在したデータについてはコールバックは呼び出されません。
Member名がわかっていれば初回のClient::sync()前に設定すればよいです。
そうでなければClient::onMemberEntry()イベントのコールバックの中で各種イベントを設定すればよいです。

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.member("foo").value("hoge").appendListener([](webcface::Value v){ /* ... */ });
    wcli.member("foo").onSync().appendListener([](webcface::Member m){ /* ... */ });
    ```
    例えば全Memberの全Valueデータを受信するには
    ```cpp
    wcli.onMemberEntry().appendListener([](webcface::Member m){
        m.onValueEntry().appendListener([](webcface::Value v){
            v.appendListener([](webcface::Value v){
                // ...
            });
        });
    });
    ```
    のようにすると可能です。
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Member, Value } from "webcface";
    wcli.member("foo").value("hoge").on((v: Value) => { /* ... */ });
    wcli.member("foo").onSync.on((m: Member) => { /* ... */ });
    ```
    例えば全Memberの全Valueデータを受信するには
    ```ts
    wcli.onMemberEntry.on((m: Member) => {
        m.onValueEntry.on((v: Value) => {
            v.on((v: Value) => {
                // ...
            });
        });
    });
    ```
    のようにすると可能です。
- <b class="tab-title">Python</b>
    pythonでは Value.signal プロパティがこのイベントのsignalを返します。
    ```python
    def value_change(v: webcface.Value):
        pass
    wcli.member("foo").value("hoge").signal.connect(value_change)
    def synced(m: webcface.Member):
        pass
    wcli.member("foo").on_sync.connect(synced)
    ```

</div>

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Member](02_member.md) | [Text](11_text.md) |

</div>
