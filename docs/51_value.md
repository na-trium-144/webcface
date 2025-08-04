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

数値データ、または数値の配列データを送受信する型です。
double型またはそのリストとして送受信されます。

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
    Value::set() でデータを代入し、Client::sync()することで送信されます。

    ```cpp
    wcli.value("hoge").set(5);
    wcli.value("fuga").set({1, 2, 3, 4, 5});
    ```

    set() の代わりに代入演算子(Value::operator=)でも同様のことができます。
    また、 operator+= など、doubleやintの変数で使える各種演算子も使えます
    ```cpp
    wcli.value("hoge") = 5;
    ```

    \note
    * webcfaceのvalueはset(),get()関数のインタフェースとしてはdouble型のみが用意されていますが、値が整数だった場合自動的に整数型に変換して送受信するため、整数値やbool値を送りたい場合でも通信量を気にする必要はありません。
    * <span class="since-c">1.7</span>
    配列データは`std::vector<double>`だけでなく、std::arrayや生配列などstd::ranges::rangeに合うものならなんでも使えます。
    要素の型はdoubleに変換可能ならなんでもokです。
    * <span class="since-c">2.10</span>
    要素数0の配列を渡した場合、要素数1で値が0になります。

    <span class="since-c">1.11</span>
    valueに直接`[]` <del>(または`child()`)</del> (<span class="since-c">2.8</span> または `at()`) で配列の要素アクセスが可能です。
    また、resize() で配列を初期化し push_back() で追加する使い方もできます。
    ただしValueにセットしたデータはsync等で自動的にリセットされることはないので、push_backとsyncを繰り返す場合は resize(0) や set({}) などで初期化してください。
    ```cpp
    wcli.value("fuga").resize(5);
    for(int i = 0; i < 5; i++){
        wcli.value("fuga")[i] = i;
        // wcli.value("fuga").at(i) も同じ
    }
    wcli.value("fuga").push_back(100);
    ```
    <span class="since-c">2.8</span>
    `[]`や`at()`は ValueElementRef 型を返すようになりました。使い方は今までと変わりません。

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

    \note
    * webcfaceのvalueはset(),get()関数のインタフェースとしてはdouble型のみが用意されていますが、値が整数だった場合自動的に整数型に変換して送受信するため、整数値やbool値を送りたい場合でも通信量を気にする必要はありません。
    * <span class="since-c">2.10</span>
    要素数0の配列を渡した場合、要素数1で値が0になります。

- <b class="tab-title">JavaScript</b>
    Value.set() でデータを代入し、Client.sync()することで送信されます。
    ```ts
    wcli.value("hoge").set(5);
    wcli.value("fuga").set([1, 2, 3, 4, 5]);
    ```

- <b class="tab-title">Python</b>
    Value.set() でデータを代入し、Client.sync()することで送信されます。
    ```python
    wcli.value("hoge").set(5)
    wcli.value("fuga").set([1, 2, 3, 4, 5])
    ```
    
    <span class="since-py">2.0</span>
    数値型はfloatに変換可能なもの(SupportsFloat型)であればなんでもokです。(例えばnumpyの数値型なども使用可能です。)

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
    Value::tryGet(), Value::tryGetVec() などで値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```cpp
    std::optional<double> hoge = wcli.member("foo").value("hoge").tryGet();
    std::optional<webcface::NumVector> hoge = wcli.member("foo").value("hoge").tryGetVec();
    ```
    * 値をまだ受信していない場合 tryGet(), tryGetVec() はstd::nulloptを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient::sync()したときに</del>
        <span class="since-c">1.2</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * Value::get(), Value::getVec() はstd::nulloptの代わりに0を返します。
    * <span class="since-c">2.10</span> tryGetVec(), getVec() は webcface::NumVector 型で配列データを返します。 (以前は std::vector\<double\> を返していた)
        * NumVectorはshared_ptrで配列データの参照を保持しており、コピー時にコストがかかりません。
        * std::vector\<double\>にキャストすることも可能です。
    * また、Valueを直接 double や NumVector, std::vector\<double\> などの型にキャストすることでも同様に値が得られます。

    <span class="since-c">1.7</span>
    Value::request() で明示的にリクエストを送信することもできます。

    <span class="since-c">1.8</span>
    std::ostreamにValueを直接渡して表示することもできます。
    まだ受信していない場合nullと表示されます。
    ```cpp
    std::cout << "hoge = " << wcli.member("foo").value("hoge") << std::endl;
    ```

    <span class="since-c">1.11</span>
    送信時と同様、配列データは`[]` <del>または`child()`</del> <span class="since-c">2.8</span>または`at()` を使ってもアクセスできます。
    ```cpp
    double hoge = wcli.member("foo").value("hoge")[3].get();
    ```

    <span class="since-c">2.8</span>
    `size()` で配列データのサイズを取得できます。
    ただし tryGet() などと同様、まだ受信していないデータに関しては0を返し、リクエストが送られます。
    
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

</div>

## Entry, イベントについて

[4-3. Field](43_field.md) に移動しました。そちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [4-3. Field](43_field.md) | [5-2. Text](52_text.md) |

</div>
