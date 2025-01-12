# 5-1. Value

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
double型で送受信されます。

## コマンドライン

```sh
webcface-send 
```
を実行し、数字を入力すると送信されます。(1つ入力するごとに改行してください)

オプションでclientやデータの名前を変更できます。
詳細は [webcface-send](./72_send.md) のページを参照

## 送信

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::value からValueオブジェクトを作り、 Value::set() でデータを代入し、Client::sync()することで送信されます。

    ```cpp
    wcli.value("hoge").set(5);
    wcli.value("fuga").set({1, 2, 3, 4, 5});
    ```
    \note
    <span class="since-c">1.7</span>
    配列データは`std::vector<double>`だけでなく、std::arrayや生配列などstd::ranges::rangeに合うものならなんでも使えます。
    要素の型はdoubleに変換可能ならなんでもokです。

    set() の代わりに代入演算子(Value::operator=)でも同様のことができます。
    また、 operator+= など、doubleやintの変数で使える各種演算子も使えます
    ```cpp
    wcli.value("hoge") = 5;
    ```

    <span class="since-c">1.11</span>
    valueに直接`[]`(または`child()`)で要素アクセスが可能です。
    また、resize()で配列を初期化し push_back() (<span class="since-c">2.6</span> pushBack() も可) で追加する使い方もできます。
    ```cpp
    wcli.value("fuga").resize(5);
    for(int i = 0; i < 5; i++){
        wcli.value("fuga").push_back(i);
    }
    wcli.value("fuga")[3] = 100; // 上書き
    // wcli.value("fuga").child(3) = 100; としても同じ
    ```

- <b class="tab-title">C</b>
    double型の単一の値は wcfValueSet, (<span class="since-c">2.0</span> wcfValueSetW)
    ```c
    wcfValueSet(wcli, "hoge", 123.45);
    ```
    配列データは wcfValueSetVecD, (<span class="since-c">2.0</span> wcfValueSetVecDW)
    ```c
    double value[5] = {1, 2, 3, 4, 5};
    wcfValueSetVecD(wcli, "fuga", value, 5);
    ```
    で送信できます。

- <b class="tab-title">JavaScript</b>
    Client.value からValueオブジェクトを作り、 Value.set() でデータを代入し、Client.sync()することで送信されます。
    ```ts
    wcli.value("hoge").set(5);
    wcli.value("fuga").set([1, 2, 3, 4, 5]);
    ```

- <b class="tab-title">Python</b>
    Client.value からValueオブジェクトを作り、 Value.set() でデータを代入し、Client.sync()することで送信されます。
    ```python
    wcli.value("hoge").set(5)
    wcli.value("fuga").set([1, 2, 3, 4, 5])
    ```
    
    <span class="since-py">2.0</span>
    数値型はfloatに変換可能なもの(SupportsFloat型)であればなんでもokです。(例えばnumpyの数値型なども使用可能です。)

</div>

\note
(serverが<span class="since-c">1.10</span>以降の場合)
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

Valueに限らず他のデータ型 ([View](./54_view.md), [Canvas2D](./61_canvas2d.md), [Image](./62_image.md), [Canvas3D](./63_canvas3d.md), [RobotModel](./64_robot_model.md)) でも同様です。

![value_child](https://github.com/na-trium-144/webcface/raw/main/docs/images/value_child.png)

\note
ROSのTopicではPointやTransformなど目的に応じてさまざまな型が用意されていますが、
WebCFaceではそういう場合はValueを複数用意して送信することを想定しています。

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

<details><summary>(ver1.11まで) C++でwebcface::Value::Dictを使った値のセット</summary>

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

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::value() でValueクラスのオブジェクトが得られ、
    Value::tryGet(), Value::tryGetVec() などで値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```cpp
    std::optional<double> hoge = wcli.member("foo").value("hoge").tryGet();
    std::optional<std::vector<double>> hoge = wcli.member("foo").value("hoge").tryGetVec();
    ```
    * 値をまだ受信していない場合 tryGet(), tryGetVec() はstd::nulloptを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient::sync()したときに</del>
        <span class="since-c">1.2</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * Value::get(), Value::getVec() はstd::nulloptの代わりに0を返します。
    * また、doubleやstd::vector\<double\> などの型にキャストすることでも同様に値が得られます。

    <span class="since-c">1.7</span>
    Value::request() で明示的にリクエストを送信することもできます。

    <span class="since-c">1.8</span>
    std::ostreamにValueを直接渡して表示することもできます。
    まだ受信していない場合nullと表示されます。
    ```cpp
    std::cout << "hoge = " << wcli.member("foo").value("hoge") << std::endl;
    ```

    グループ化したデータは送信時と同様child()を使っても要素を参照できます。
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
    * <span class="since-c">1.11</span>
    Valueオブジェクト同士を `==`, `!=` で比較するとValueが参照するデータの名前が一致するかどうかで判定されます。
    (Textなど他のデータ型でも同様です。)  
    * ver1.10まではdoubleにキャストされた上で値を比較していたので、異なる挙動になります。
    値を比較したい場合は明示的にキャストするか`get()`などを呼んでください。

    <span></span>

- <b class="tab-title">C</b>
    wcfValueGetVecD, (<span class="since-c">2.0</span> wcfValueGetVecDW) で受信できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。
    ```c
    double value[5];
    int size;
    int ret = wcfValueGetVecD(wcli, "foo", "hoge", value, 5, &size);
    // ex.) ret = WCF_NO_DATA

    // after wcfSync(),
    ret = wcfValueGetVecD(wcli, "foo", "hoge", value, 5, &size);
    // ex.) ret = WCF_OK, value = {123.45, 0, 0, 0, 0}, size = 1
    ```
    * sizeに受信した値の個数、valueに受信した値が入ります。
    * 値をまだ受信していない場合 <del>`WCF_NOT_FOUND`</del>
    <span class="since-c">2.0</span> `WCF_NO_DATA` を返し、そのデータのリクエストをサーバーに送ります。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度wcfValueGetVecD()することで値が得られます。

    <span class="since-c">1.7</span>
    1つの値のみを受信する場合は wcfValueGet, (<span class="since-c">2.0</span> wcfValueGetW) も使えます。
    double型1つのみが受け取れる以外、動作はwcfValueGetVecDと同様です。
    ```c
    double value;
    ret = wcfValueGet(wcli, "a", "hoge", &value);
    // ex.) value = 123.45
    ```

    \note <span class="since-c">1.7</span> member名に空文字列またはNULLを指定すると自分自身を指します。

- <b class="tab-title">JavaScript</b>
    Member.value() でValueクラスのオブジェクトが得られ、
    Value.tryGet(), Value.tryGetVec() などで値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```ts
    const hoge: double | null = wcli.member("foo").value("hoge").tryGet();
    const hoge: double[] | null = wcli.member("foo").value("hoge").tryGetVec();
    ```
    * 値を受信していない場合 tryGet(), tryGetVec() はnullを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient.sync()したときに</del>
        <span class="since-js">1.1</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * get(), getVec() はnullの代わりに0を返します。

    <span class="since-js">1.1</span>
    Value.request()で明示的にリクエストを送信することもできます。

- <b class="tab-title">Python</b>
    Member.value() でValueクラスのオブジェクトが得られ、
    Value.try_get(), Value.try_get_vec() などで値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```python
    # float型 or None
    hoge = wcli.member("foo").value("hoge").try_get()
    # List[float] or None
    hoge = wcli.member("foo").value("hoge").try_get_vec()
    ```
    * 値をまだ受信していない場合 try_get(), try_get_vec() はNoneを返し、そのデータのリクエストをサーバーに送ります。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度try_get()することで値が得られます。
    * get(), getVec() はNoneの代わりに0を返します。

    Value.request()で明示的にリクエストを送信することもできます。

    グループ化したデータは送信時と同様child()を使っても要素を参照できます。
    ```cpp
    pos = wcli.member("foo").value("pos");
    x = pos.child("x").get(); // = "pos.x"
    y = pos.child("y").get(); // = "pos.y"
    z = pos.child("z").get(); // = "pos.z"
    ```

</div>

### 時刻

<div class="tabbed">

- <b class="tab-title">C++</b>
    <del>Value::time()</del> でその値が送信されたとき(そのMemberがsync()で送信したとき)の時刻が得られます。  
    <span class="since-c">1.7</span>
    Member::syncTime() に変更
    (Textなど他のデータの送信時刻と共通です)

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    wcfMemberSyncTime() でその値が送信されたとき(そのMemberがsync()で送信したとき)の時刻が得られます。
    (Textなど他のデータの送信時刻と共通です)

- <b class="tab-title">JavaScript</b>
    <del>Value.time()</del> でその値が送信されたとき(そのMemberがsync()で送信したとき)の時刻が得られます。  
    <span class="since-js">1.6</span>
    Member.syncTime() に変更
    (Textなど他のデータの送信時刻と共通です)

- <b class="tab-title">Python</b>
    Member.sync_time() でその値が送信されたとき(そのMemberがsync()で送信したとき)の時刻が得られます。
    (Textなど他のデータの送信時刻と共通です)

</div>

### Entry

データ自体はすべて受信しなくても、データが存在するかどうか(他memberが送信しているかどうか)は取得することができます。

\warning
(serverが<span class="since-c">1.10</span>以降の場合)
半角ピリオドから始まる名前のデータはEntryが送信されないため、
明示的に名前を指定して受信することはできても、以下の方法でデータが存在するかどうかを確認することはできません。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <del>Member::values() で</del> そのMemberが送信しているvalueのリストが得られます  
    <span class="since-c">1.6</span>
    Member::valueEntries() に変更

    ```cpp
    for(const webcface::Value &v: wcli.member("foo").valueEntries()){
        // ...
    }
    ```

    [Text](52_text.md), [Func](53_func.md), [View](54_view.md)
    など他のデータ型に関しても同様に `textEntries()`, `funcEntries()`, `viewEntries()` などで取得できます。

    <span class="since-c">1.11</span>
    Field::valueEntries() でそのfield以下のvalueのみが得られます
    (Textなど他の型についても同様)
    ```cpp
    std::vector<webcface::Value> values = wcli.member("foo").field("pos").valueEntries();
    // pos.x, pos.y などのvalueが得られる
    ```

    <span class="since-c">2.1</span>
    Value::exists() でそのデータが送信されているかどうかを確認できます。
    tryGet() と違い、データそのものを受信するリクエストは送られません。
    他のデータ型に関しても同様に `Text::exists()`, `Func::exists()`, `View::exists()` などが使えます。

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfValueEntryList`, `wcfValueEntryListW` にchar\*の配列とサイズを渡すと、valueの一覧を取得できます。
    ```c
    const char *value_list[10];
    int actual_value_num;
    wcfValueEntryList(wcli, "foo", value_list, 10, &actual_value_num);
    ```
    それぞれのvalue名の文字列は、 wcfClose() するまではfreeされません。

    [Text](52_text.md), [Func](53_func.md), [View](54_view.md)
    など他のデータ型に関しても同様に `wcfTextEntryList()`, `wcfFuncEntryList()`, `wcfViewEntryList()` などで取得できます。


- <b class="tab-title">JavaScript</b>
    Member.values() でそのMemberが送信しているvalueのリストが得られます  
    ```js
    for(const v of wcli.member("foo").values()){
        // ...
    }
    ```

    [Text](52_text.md), [Func](53_func.md), [View](54_view.md)
    など他のデータ型に関しても同様に `texts()`, `funcs()`, `views()` などで取得できます。

    <span class="since-js">1.8</span>
    Value.exists() でそのデータが送信されているかどうかを確認できます。
    tryGet() と違い、データそのものを受信するリクエストは送られません。
    他のデータ型に関しても同様に `Text.exists()`, `Func.exists()`, `View.exists()` などが使えます。

- <b class="tab-title">Python</b>
    <del>Member.values() で</del> そのMemberが送信しているvalueのリストが得られます  
    <span class="since-py">1.1</span>
    Member.value_entries() に変更

    ```python
    for v in wcli.member("foo").value_entries():
        # ...
    ```

    [Text](52_text.md), [Func](53_func.md), [View](54_view.md)
    など他のデータ型に関しても同様に `text_entries()`, `func_entries()`, `view_entries()` などで取得できます。

    <span class="since-py">2.0</span>
    Value.exists() でそのデータが送信されているかどうかを確認できます。
    try_get() と違い、データそのものを受信するリクエストは送られません。
    他のデータ型に関しても同様に `Text.exists()`, `Func.exists()`, `View.exists()` などが使えます。

</div>

### ValueEntry イベント

他のメンバーが新しくデータを追加したときに呼び出されるコールバックを設定できます。

イベントの詳細な使い方はMemberEntryと同様です([Member](./42_member.md) のページを参照してください)。
このクライアントが接続する前から存在したデータについては start(), waitConnection() 時に一度に送られるので、
コールバックの設定はstart()より前に行うと良いです。

ValueEntryではデータの存在を知ることしかできません。
データの内容を取得するにはコールバックの中で改めてget()やrequest()を呼ぶか、
後述のValueChangeイベントを使ってください。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <span class="since-c">2.0</span>
    Member::onValueEntry() でコールバックを設定できます。
    新しく追加されたValueの情報が引数に渡されます。
    ```cpp
    wcli.member("foo").onValueEntry([](webcface::Value v){ /* ... */ });
    ```
    ver1.11以前では `.onValueEntry().appendListener(...)`

    他のデータ型に関しても同様に `onTextEntry()`, `onFuncEntry()`, `onViewEntry()` などが使えます。

    \note
    * コールバックを設定する前から存在したデータについてはコールバックは呼び出されません。
    * <span class="since-c">2.0</span>
    Client::waitConnection()は接続時にサーバーに存在するデータすべてについてコールバックを呼んでからreturnします。
    * すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
    Member名がわかっていれば<del>初回の Client::sync()</del> Client::start() または waitConnection() 前に設定してください。
    * すべてのメンバーのすべてのデータに対してコールバックが呼ばれるようにしたい場合は、 Client::onMemberEntry() イベントのコールバックの中で各種イベントを設定すればよいです。

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfValueEntryEvent`, `wcfValueEntryEventW` で引数に const char \* 2つと void \* をとる関数ポインタをコールバックとして設定できます。  
    新しく追加されたValueの名前が引数に渡されます。
    void\*引数には登録時に任意のデータのポインタを渡すことができます。(使用しない場合はNULLでよいです。)
    ```c
    void callback_value_entry(const char *member_name, const char *value_name, void *user_data_p) {
        // member_name is "foo"

        struct UserData *user_data = (struct UserData *)user_data_p;
        // ...
    }
    struct UserData user_data = {...};
    wcfValueEntryEvent(wcli, "foo", callback_value_entry, &user_data);
    ```

    他のデータ型に関しても同様に `wcfTextEntryEvent()`, `wcfFuncEntryEvent()`, `wcfViewEntryEvent()` などが使えます。

    \note
    * コールバックを設定する前から存在したデータについてはコールバックは呼び出されません。
    * wcfWaitConnection()は接続時にサーバーに存在するデータすべてについてコールバックを呼んでからreturnします。
    * すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
    Member名がわかっていれば wcfStart() または wcfWaitConnection() 前に設定してください。
    * すべてのメンバーのすべてのデータに対してコールバックが呼ばれるようにしたい場合は、 MemberEntryイベントのコールバックの中で各種イベントを設定すればよいです。

- <b class="tab-title">JavaScript</b>
    Member.onValueEntry でコールバックを設定できます。
    新しく追加されたValueの情報が引数に渡されます。
    ```ts
    import { Value } from "webcface";
    wcli.member("foo").onValueEntry.on((v: Value) => { /* ... */ });
    ```

    他のデータ型に関しても同様に `onTextEntry`, `onFuncEntry`, `onViewEntry` などが使えます。

    \note
    * コールバックを設定する前から存在したデータについてはコールバックは呼び出されません。
    * すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
    Member名がわかっていれば Client.start() 前に設定してください。
    * すべてのメンバーのすべてのデータに対してコールバックが呼ばれるようにしたい場合は、 Client.onMemberEntry イベントのコールバックの中で各種イベントを設定すればよいです。

- <b class="tab-title">Python</b>
    Member.on_value_entry() でコールバックを設定できます。
    新しく追加されたValueの情報が引数に渡されます。
    ```python
    def value_entry(v: webcface.Value):
        pass
    wcli.member("foo").on_value_entry(value_entry)
    ```
    * ver1.1以前では `on_value_entry.connect(...)`
    * 他のデータ型に関しても同様に `on_text_entry()`, `on_func_entry()`, `on_view_entry()` などが使えます。
    * on_member_entryと同様、デコレータにすることもできます。

    \note
    * コールバックを設定する前から存在したデータについてはコールバックは呼び出されません。
    * <span class="since-py">2.0</span>
    Client.wait_connection()は接続時にサーバーに存在するデータすべてについてコールバックを呼んでからreturnします。
    * すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
    Member名がわかっていればClient.start() または wait_connection() 前に設定してください。
    * すべてのメンバーのすべてのデータに対してコールバックが呼ばれるようにしたい場合は、 Client.on_member_entry() イベントのコールバックの中で各種イベントを設定すればよいです。

    <span></span>

</div>

### ValueChange イベント, onSync イベント

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定するとget()やrequest()を呼ばなくても自動的にその値がリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は onSync が使えます。

イベントの詳細な使い方はonMemberEntryと同様です([Member](./42_member.md) のページを参照してください)。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <span class="since-c">2.0</span>
    Value::onChange(), Member::onSync() でコールバックを設定できます。  
    引数にはそれぞれそのValue自身,Member自身が渡されます。
    (キャプチャでも同じことができるのでなくてもよい)
    ```cpp
    wcli.member("foo").value("hoge").onChange([](webcface::Value v){ /* ... */ });
    wcli.member("foo").onSync([](webcface::Member m){ /* ... */ });
    ```
    * ver1.11以前は `value("hoge").appendListener(...)`, `member("foo").onSync().appendListener(...)` です
    * <span class="since-c">1.7</span>
    引数を持たない関数もイベントのコールバックに設定可能です。
    ```cpp
    wcli.member("foo").value("hoge").onChange([](){ /* ... */ });
    wcli.member("foo").onSync([](){ /* ... */ });
    ```
    (ver1.11以前は appendListener())

    すべてのデータを受信したい場合は ValueEntry イベントの中でonChangeを設定すると可能です。
    ```cpp
    wcli.onMemberEntry([](webcface::Member m){
        m.onValueEntry([](webcface::Value v){
            v.onChange([](webcface::Value v){
                // ...
            });
        });
    });
    ```


- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfValueChangeEvent`, `wcfValueChangeEventW` で引数に const char \* 2つと void \* をとる関数ポインタをコールバックとして設定できます。
    void\*引数には登録時に任意のデータのポインタを渡すことができます。(使用しない場合はNULLでよいです。)
    ```c
    void callback_value_change(const char *member_name, const char *value_name, void *user_data_p) {
        // member_name is "foo", value_name is "hoge"
        
        // struct UserData *user_data = (struct UserData *)user_data_p;
        // ...

        double value[5];
        int size;
        wcfValueGetVecD(wcli, member_name, value_name, value, 5, &size);

    }
    struct UserData user_data = {...};
    wcfValueChangeEvent(wcli, "foo", "hoge", callback_value_change, &user_data);
    ```

- <b class="tab-title">JavaScript</b>
    Value.on(), Member.onSync.on() などでコールバックを設定できます。  
    引数にはそれぞれそのValue自身,Member自身が渡されます。
    (なくてもよい)

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
    <span class="since-py">2.0</span>
    Value.on_change(), Member.on_sync() でコールバックを設定できます。
    引数にはそれぞれそのValue自身,Member自身が渡されます。

    ```python
    def value_change(v: webcface.Value):
        pass
    wcli.member("foo").value("hoge").on_change(value_change)
    def synced(m: webcface.Member):
        pass
    wcli.member("foo").on_sync(synced)
    ```

    すべてのデータを受信したい場合は ValueEntry イベントの中でonChangeを設定すると可能です。
    ```python
    @wcli.on_member_entry
    def member_entry(m: Member):
        @m.on_value_entry
        def value_entry(v: Value):
            @v.on_change
            def on_change(v: Value):
                pass
    ```

</div>

<details><summary>Python 〜ver1.1の仕様</summary>

pythonでは Value.signal プロパティがこのイベントのsignalを返します。
```python
def value_change(v: webcface.Value):
    pass
wcli.member("foo").value("hoge").signal.connect(value_change)
def synced(m: webcface.Member):
    pass
wcli.member("foo").on_sync.connect(synced)
```

</details>

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [4-2. Member](42_member.md) | [5-2. Text](52_text.md) |

</div>
