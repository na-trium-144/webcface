# Value

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::Value (`webcface/value.h`)
* C Reference: c_wcf/value.h
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

    <span class="since-c">1.11</span>
    valueに直接`[]`(または`child()`)で要素アクセスが可能です。
    また、resize()で配列を初期化しpush_back()で追加する使い方もできます。
    ```cpp
    wcli.value("fuga").resize(5);
    for(int i = 0; i < 5; i++){
        wcli.value("fuga").push_back(i);
    }
    wcli.value("fuga")[3] = 100; // 上書き
    // wcli.value("fuga").child(3) = 100; としても同じ
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

\warning
<span class="since-c">1.10</span>
データの名前を半角ピリオドから始めると、Entryが他クライアントに送信されなくなります。
(WebUI上に表示することなくデータを送ることができます)  
半角ピリオド2つから始まる名前はwebcface内部の処理で利用する場合があるので使用しないでください。  
Text、Funcなど他のデータ型についても同様です。

### 通信データについて

クライアントが送信したデータは、サーバーを経由して別のクライアントに送られます。
(Valueに限らず、これ以降説明する他のデータ型のfieldについても同様です。)

![pub-sub](https://github.com/na-trium-144/webcface/raw/main/docs/images/pub-sub.png)

サーバー→クライアント間では、初期状態ではデータは送信されず、
クライアントがリクエストを送って初めてサーバーからデータが順次送られてくるようになります。

<span class="since-c">1.8</span>
<span class="since-js">1.4.1</span>
<span class="since-py">1.1.2</span>
クライアント→サーバー間では、同じデータを繰り返しsetした場合は2回目以降はデータを送信しないことで通信量を削減しています。

基本的にクライアント→サーバーの方向にはすべてのデータが送信されるのに対し、サーバー→クライアントの方向には必要なデータのみを送信する設計になっていますが、これは前者はlocalhost(サーバーとクライアントが同じPC)のみで、後者はWi-FiやLANでも通信することを想定したものです。
(通信量は増えますがクライアント→サーバーのデータ送信をWi-FiやLAN経由で行うことも可能です)

\note
webcfaceのvalueは浮動小数型のみを扱いますが、値が整数だった場合シリアライズ時に自動的に整数型として送受信されるようなので、整数値やbool値を送りたい場合でも通信量を気にする必要はありません。([msgpack/msgpack-c#1017](https://github.com/msgpack/msgpack-c/issues/1017))

### グループ化

Valueの名前を半角ピリオドで区切ると、WebUI上ではフォルダアイコンで表示されグループ化されて表示されます。

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
    pos.child("x") = 1; // = "pos.x"
    pos.child("y") = 2; // = "pos.y"
    pos.child("z") = 3; // = "pos.z"
    ```

    <span class="since-c">1.11</span>
    Member::child() で webcface::Field 型としてオブジェクトが得られ、そこからvalue()でValue型に変換することもできます。
    (この場合はValue以外の型に変換することもでき汎用性が高いです)
    また、`[]`を使ってもchild()と同じ結果になります。
    ```cpp
    webcface::Field pos = wcli.child("pos");
    pos.child("x").value() = 1; // = "pos.x"
    pos["y"].value() = 2;       // = "pos.y"
    pos.value("z") = 3;         // = "pos.z"
    ```

    \note <span class="since-c">1.11</span>
    child() (または`[]`)の引数が数値(または`"1"`のような文字列でも同じ)の場合、
    グループ化ではなく配列としての値代入が優先されます。
    (これはValue型のみの特別な処理です。)
    ただし以下のような場合は通常の文字列と同様に処理します。
    ```cpp
    wcli.value("data")[0]["a"] = 1; // value("data.0.a") = 1
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

### 複数の値をまとめて送る

\todo
Pythonの辞書型への対応は未実装

<div class="tabbed">

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

<details><summary>(deprecated) C++でwebcface::Value::Dictを使った値のセット</summary>

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

</details>

## 受信

Member::value() でValueクラスのオブジェクトが得られ、
Value::tryGet(), Value::tryGetVec() などで値のリクエストをするとともに受信した値を取得できます。

例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    std::optional<double> hoge = wcli.member("foo").value("hoge").tryGet();
    std::optional<std::vector<double>> hoge = wcli.member("foo").value("hoge").tryGetVec();
    ```
    値を受信していない場合 tryGet(), tryGetVec() はstd::nulloptを返します。  
    get(), getVec() はstd::nulloptの代わりにデフォルト値を返します。  
    また、doubleやstd::vector<double> などの型にキャストすることでも同様に値が得られます。

    <span class="since-c">1.8</span>
    std::ostreamにValueを直接渡して表示することもできます。
    まだ受信していない場合nullと表示されます。
    ```cpp
    std::cout << "hoge = " << wcli.member("foo").value("hoge") << std::endl;
    ```

    グループ化したデータは送信時と同様child()を使って要素を参照できます。
    ```cpp
    webcface::Value pos = wcli.member("foo").value("pos");
    double x = pos.child("x").get(); // = "pos.x"
    double y = pos.child("y").get(); // = "pos.y"
    double z = pos.child("z").get(); // = "pos.z"
    ```
    ```cpp
    webcface::Field pos = wcli.member("foo").child("pos");
    double x = pos.child("x").value().get(); // = "pos.x"
    double y = pos["y"].value().get();       // = "pos.y"
    double z = pos.value("z").get();         // = "pos.z"
    ```

    <span class="since-c">1.11</span>
    送信時と同様、配列データはchild()または`[]`を使ってもアクセスできます。
    ```cpp
    double hoge = wcli.member("foo").value("hoge")[3].get();
    ```

    \warning
    <span class="since-c">1.11</span>
    Valueオブジェクト同士を比較するとValueが参照するデータの名前が一致するかどうかで判定されます。(Textなど他のデータ型でも同様です。)  
    ver1.10まではdoubleにキャストされた上で値を比較していたので、異なる挙動になります。
    値を比較したい場合は明示的にキャストするか`get()`などを呼んでください。

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

    値を受信していない場合`WCF_NOT_FOUND`を返し、別スレッドでリクエストが送信されます。
    
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
    値を受信していない場合 tryGet(), tryGetVec() はnullを返します。  
    get(), getVec() はnullの代わりにデフォルト値を返します。
- <b class="tab-title">Python</b>
    ```python
    hoge = wcli.member("foo").value("hoge").try_get()
    hoge = wcli.member("foo").value("hoge").try_get_vec()
    ```
    値を受信していない場合 try_get(), try_get_vec() はNoneを返します。  
    get(), getVec() はNoneの代わりにデフォルト値を返します。

</div>

### リクエスト
get()などの初回の呼び出しではまだ値を受信していないためnullなどを返しますが、  
~~Client::sync()したときに実際にリクエストが送信され、~~  
<span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
自動的に別スレッドでリクエストが送信され、サーバーから値が返ってきたら値が得られるようになります。
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
<span class="since-js">1.6</span>
<span class="since-py"></span>
Member::syncTime() に変更
(Textなど他のデータの送信時刻と共通です)

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

Member::onValueEntry() で新しくデータが追加されたときのコールバックを設定できます。

ただし、コールバックを設定する前から存在したデータについてはコールバックは呼び出されません。
すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
Member名がわかっていれば初回の Client::sync() 前に、
そうでなければ Client::onMemberEntry() イベントのコールバックの中で各種イベントを設定すればよいです。

イベントの詳細な使い方はonMemberEntryと同様です([Member](./02_member.md) のページを参照してください)。

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

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます。

イベントの詳細な使い方はonMemberEntryと同様です([Member](./02_member.md) のページを参照してください)。

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
