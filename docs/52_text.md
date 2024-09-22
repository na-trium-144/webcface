# 5-2. Text

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::Text  (`webcface/text.h`)
* C Reference: c_wcf/text.h
* JavaScript [Text](https://na-trium-144.github.io/webcface-js/classes/Text.html)
* Python [webcface.Text](https://na-trium-144.github.io/webcface-python/webcface.text.html#webcface.text.Text)

文字列データを送受信します。

使い方は[Value](./51_value.md)とほぼ同じです。

## コマンドライン

```sh
webcface-send -t text
```
を実行し、文字列を入力すると送信されます。(1つ入力するごとに改行してください)

詳細は [webcface-send](./72_send.md) のページを参照

## 送信

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::text からTextオブジェクトを作り、 Text::set() でデータを代入し、Client::sync()することで送信されます

    ```cpp
    wcli.text("hoge").set("hello");
    ```
    set() の代わりに代入演算子(Text::operator=)でも同様のことができます。
    ```cpp
    wcli.text("hoge") = "hello";
    ```

- <b class="tab-title">C</b>
    \since <span class="since-c">1.7</span>

    null終端の文字列は wcfTextSet, (<span class="since-c">2.0</span> wcfTextSetW) で送信できます
    ```c
    wcfTextSet(wcli, "hoge", "hello");
    ```
    null終端でない場合は wcfTextSetN, (<span class="since-c">2.0</span> wcfTextSetNW) が使えます
    ```c
    wcfTextSetN(wcli, "hoge", "hello", 5);
    ```

- <b class="tab-title">JavaScript</b>
    Client.text からTextオブジェクトを作り、 Text.set() でデータを代入し、Client.sync()することで送信されます。
    ```ts
    wcli.text("hoge").set("hello");
    ```

- <b class="tab-title">Python</b>
    Client.text からTextオブジェクトを作り、 Text.set() でデータを代入し、Client.sync()することで送信されます。
    ```python
    wcli.text("hoge").set("hello")
    ```

</div>

\note
* <span class="since-c">1.10</span>
<span class="since-js">1.6</span>
<span class="since-py">2.0</span>
Textの内部データは文字列だけでなく数値やbool値も型を保持して扱えるようになっています。
([View](./54_view.md)のInputRefで内部的に使用するため)
* <del>C++では Text::set, Text::get で ValAdaptor 型を経由して文字列以外の型のデータを文字列と同様に送信、受信できます。</del>
    * <span class="since-c">2.0</span> 文字列以外の型にアクセスするインタフェースは Variant という別のクラスに分離しました。
    Textクラスから文字列以外のデータ型にはアクセスできません。
* PythonもC++と同様Variantクラスからアクセスできます。
* JavaScriptでは Text.setAny, Text.getAny 関数を使うと文字列以外のデータを処理できます。

<!--Valueと同様名前に半角ピリオドを含めると、WebUI上ではフォルダアイコンで表示されグループ化されて表示されます。-->

<details><summary>(deprecated, ver1.10で削除) C++でwebcface::Text::Dictを使った値のセット</summary>

webcface::Text::Dict オブジェクトを使うと複数の値をまとめて送ることができます。
```cpp
struct A {
    std::string x, y;
    operator webcface::Text::Dict() const {
        return {
            {"x", x},
            {"y", y},
            // Value::Dictと同様、入れ子にもできます
        }
    }
};

A a_instance;
wcli.text("a").set(a_instance); // Dictにキャストされる
```

</details>

<!--
- <b class="tab-title">JavaScript</b>
    オブジェクトを渡すことができます。
    ```ts
    wcli.text("a").set({
        x: "aaa",
        y: "bbb",
        // Value::Dictと同様、入れ子にもできます
    });
    ```

</div>
-->

## 受信

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::text() でTextクラスのオブジェクトが得られ、
    Text::tryGet() で値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```cpp
    std::optional<std::string> hoge = wcli.member("foo").text("hoge").tryGet();
    ```
    * 値をまだ受信していない場合 tryGet() はstd::nulloptを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient::sync()したときに</del>
        <span class="since-c">1.2</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * Text::get() はstd::nulloptの代わりに空文字列を返します。
    * また、std::string にキャストすることでも同様に値が得られます。

    <span class="since-c">1.7</span>
    Text::request() で明示的にリクエストを送信することもできます。

    <span class="since-c">2.0</span>
    ワイド文字列は tryGetW(), getW() で得られます。
    また、std::wstring にキャストすることでも得られます。

    std::ostreamにTextを直接渡して表示することもできます。
    ```cpp
    std::cout << "hoge = " << wcli.member("foo").text("hoge") << std::endl;
    ```

    \warning
    * <span class="since-c">1.11</span>
    Textオブジェクト同士を比較するとTextが参照するデータの名前が一致するかどうかで判定されます。
    (Valueなど他のデータ型でも同様です。)  
    * 値を比較したい場合は明示的にキャストするか`get()`などを呼んでください。

    <span></span>

- <b class="tab-title">C</b>
    \since <span class="since-c">1.7</span>

    wcfTextGet, (<span class="since-c">2.0</span> wcfTextGetW)
    に受信した文字列を格納するバッファとそのサイズ(null終端を含む)を指定します。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。
    ```c
    char text[6];
    int size;
    int ret = wcfTextGet(wcli, "a", "hoge", text, 6, &size);
    // ex.) ret = WCF_NO_DATA

    // few moments later,
    ret = wcfTextGet(wcli, "a", "hoge", text, 6, &size);
    // ex.) ret = WCF_OK, text = "hello\0", size = 5
    ```
    * sizeに受信した文字列の長さ、バッファに受信した文字列が入ります。
        * 文字列がバッファの長さを超える場合は、(バッファのサイズ - 1)文字とnullが格納されます。
    * 値をまだ受信していない場合 <del>`WCF_NOT_FOUND`</del>
    <span class="since-c">2.0</span> `WCF_NO_DATA` を返し、そのデータのリクエストをサーバーに送ります。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度wcfTextGet()することで値が得られます。

    \note member名に空文字列またはNULLを指定すると自分自身を指します。

- <b class="tab-title">JavaScript</b>
    Member.text() でTextクラスのオブジェクトが得られ、
    Text.tryGet() などで値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```ts
    const hoge: string | null = wcli.member("foo").text("hoge").tryGet();
    ```
    * 値を受信していない場合 tryGet() はnullを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient.sync()したときに</del>
        <span class="since-js">1.1</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * get() はnullの代わりに空文字列を返します。

    <span class="since-js">1.1</span>
    Text.request()で明示的にリクエストを送信することもできます。

- <b class="tab-title">Python</b>
    Member.text() でTextクラスのオブジェクトが得られ、
    Text.try_get() などで値のリクエストをするとともに受信した値を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```python
    hoge = wcli.member("foo").text("hoge").try_get()
    ```
    * 値を受信していない場合 try_get() はNoneを返し、そのデータのリクエストをサーバーに送ります。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度try_get()することで値が得られます。
    * get() はNoneの代わりに空文字列を返します。

    Text.request()で明示的にリクエストを送信することもできます。

</div>

### 時刻

<del>Text::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。</del>  
<span class="since-c">1.7</span>
<span class="since-js">1.6</span>
<span class="since-py"></span>
Member::syncTime() に統一しました。詳細は [5-1. Value](./51_value.md) を参照

### Entry

Valueと同様、データ自体を受信しなくてもデータが存在するかどうかは取得することができます。
使い方は [Value](./51_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。(TextChange イベント)
コールバックを設定するとget()やrequest()を呼ばなくても自動的にその値がリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は onSync() が使えます

使い方は [Value](./51_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [5-1. Value](51_value.md) | [5-3. Func](53_func.md) |

</div>
