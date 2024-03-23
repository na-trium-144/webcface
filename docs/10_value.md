# Value

\tableofcontents
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
    \note
    <span class="since-c">1.7</span>
    配列データは`std::vector<double>`だけでなく、std::arrayや生配列などstd::ranges::rangeに合うものならなんでも使えます。
    要素の型はdoubleに変換可能ならなんでもokです。

     (C++のみ) set() の代わりに代入演算子(Value::operator=)でも同様のことができます。
    また、 operator+= など、doubleやintの変数で使える各種演算子も使えます
    ```cpp
    wcli.value("hoge") = 5;
    ```

- <b class="tab-title">C</b>
    double型の単一の値は
    ```c
    wcfValueSet(wcli, "hoge", 123.45);
    ```
    配列データは
    ```c
    double value[5] = {1, 2, 3, 4, 5};
    wcfValueSetVecD(wcli, "fuga", value, 5);
    ```
    のように送信できます。

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

\note
webcfaceのvalueは浮動小数型のみを扱いますが、値が整数だった場合シリアライズ時に自動的に整数型として送受信されるようなので通信量を気にする必要はありません。([msgpack/msgpack-c#1017](https://github.com/msgpack/msgpack-c/issues/1017))

### グループ化

Valueの名前に半角ピリオドを含めると、WebUI上ではフォルダアイコンで表示されグループ化されて表示されます。

\note
Valueに限らず他のデータ型 ([View](./13_view.md), [Canvas2D](./14_canvas2d.md), [Image](./15_image.md), [Canvas3D](./20_canvas3d.md), [RobotModel](./21_robot_model.md)) でも同様です。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.value("pos.x") = 1;
    wcli.value("pos.y") = 2;
    wcli.value("pos.z") = 3;
    ```
    Value::child() でも同じ結果になります。
    ```cpp
    webcface::Value pos = wcli.value("pos");
    pos.child("x") = 1;
    pos.child("y") = 2;
    pos.child("z") = 3;
    ```

- <b class="tab-title">C</b>
    ```c
    wcfValueSet(wcli, "pos.x", 1);
    wcfValueSet(wcli, "pos.y", 2);
    wcfValueSet(wcli, "pos.z", 3);
    ```

- <b class="tab-title">JavaScript</b>
    ```ts
    wcli.value("pos.x").set(1);
    wcli.value("pos.y").set(2);
    wcli.value("pos.z").set(3);
    ```
    Value.child() でも同じ結果になります。
    ```ts
    const pos: Value = wcli.value("pos");
    pos.child("x").set(1);
    pos.child("y").set(2);
    pos.child("z").set(3);
    ```

- <b class="tab-title">Python</b>
    ```python
    wcli.value("pos.x").set(1)
    wcli.value("pos.y").set(2)
    wcli.value("pos.z").set(3)
    ```
    Value.child() でも同じ結果になります。
    ```python
    pos = wcli.value("pos")
    pos.child("x").set(1)
    pos.child("y").set(2)
    pos.child("z").set(3)
    ```

</div>

![value_child](https://github.com/na-trium-144/webcface/raw/main/docs/images/value_child.png)

ROSのTopicではPointやTransformなど目的に応じてさまざまな型が用意されていますが、
WebCFaceではそういう場合はValueを複数用意して送信することを想定しています。

\note
<span class="since-c">1.8</span>
同じデータを繰り返しsetした場合は、通信量を削減するため実際には最初の1度しか送信されないようになっています。
([Text](./11_text.md) についても同様)

### 複数の値をまとめて送る

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
Value::tryGet(), Value::tryGetVec() などで値のリクエストをするとともに受信した値を取得できます。

例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    std::optional<double> hoge = wcli.member("foo").value("hoge").tryGet();
    std::optional<std::vector<double>> hoge = wcli.member("foo").value("hoge").tryGetVec();
    std::optional<webcface::Value::Dict> hoge = wcli.member("foo").value("hoge").tryGetRecurse();
    ```
    初回の呼び出しではまだ受信していないため、
    tryGet(), tryGetVec(), tryGetRecurse() はstd::nulloptを返します。  
    get(), getVec(), getRecurse() はstd::nulloptの代わりにデフォルト値を返します。  
    また、doubleやstd::vector<double>, Value::Dict などの型にキャストすることでも同様に値が得られます。

    <span class="since-c">1.8</span>
    std::ostreamにValueを直接渡して表示することもできます。
    まだ受信していない場合nullと表示されます。
    ```cpp
    std::cout << "hoge = " << wcli.member("foo").value("hoge") << std::endl;
    ```

- <b class="tab-title">C</b>
    ```c
    double value[5];
    int size;
    int ret = wcfValueGetVecD(wcli, "a", "hoge", value, 5, &size);
    // ex.) ret = WCF_NOT_FOUND

    // few moments later,
    ret = wcfValueGetVecD(wcli, "a", "hoge", value, 5, &size);
    // ex.) ret = WCF_OK, value = {123.45, 0, 0, 0, 0}, size = 1
    ```
    sizeに受信した値の個数、valueに受信した値が入ります。

    初回の呼び出しでは`WCF_NOT_FOUND`を返し、別スレッドでリクエストが送信されます。
    
    <span class="since-c">1.7</span>
    1つの値のみを受信する場合はwcfValueGetも使えます。
    ```c
    double value;
    ret = wcfValueGet(wcli, "a", "hoge", &value);
    ```

    \note <span class="since-c">1.7</span> member名に空文字列またはNULLを指定すると自分自身を指します。

- <b class="tab-title">JavaScript</b>
    ```ts
    const hoge: double | null = wcli.member("foo").value("hoge").tryGet();
    const hoge: double[] | null = wcli.member("foo").value("hoge").tryGetVec();
    ```
    初回の呼び出しではまだ受信していないため、
    tryGet(), tryGetVec() はnullを返します。  
    get(), getVec() はnullの代わりにデフォルト値を返します。
- <b class="tab-title">Python</b>
    ```python
    hoge = wcli.member("foo").value("hoge").try_get()
    hoge = wcli.member("foo").value("hoge").try_get_vec()
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
<span class="since-c">1.7</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
Value::request()で明示的にリクエストを送信することもできます。

### 時刻

~~Value::time()~~ でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。  
<span class="since-c">1.7</span>
<span class="since-py"></span>
Member::syncTime() に変更
(Textなど他のデータの送信時刻と共通です)

\todo JavaScriptでもMember.syncTimeに統一する

### Entry

~~Member::values() で~~ そのMemberが送信しているvalueのリストが得られます  
<span class="since-c">1.6</span>
<span class="since-py">1.1</span>
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

    <span class="since-c">1.7</span>
    引数を持たない関数もイベントのコールバックに設定可能です。
    ```cpp
    wcli.member("foo").value("hoge").appendListener([]() {
        std::cout << "foo.hoge changed" << std::endl;
    });
    ```
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
