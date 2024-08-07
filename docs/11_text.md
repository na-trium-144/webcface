# Text

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

使い方は[Value](./10_value.md)とほぼ同じです。

## コマンドライン

```sh
webcface-send -t text
```
を実行し、文字列を入力すると送信されます。(1つ入力するごとに改行してください)

詳細は [webcface-send](./71_send.md) のページを参照

## 送信

Client::text からTextオブジェクトを作り、 Text::set() でデータを代入し、Client::sync()することで送信されます

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.text("hoge").set("hello");
    ```
     (C++のみ) set() の代わりに代入演算子(Text::operator=)でも同様のことができます。
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
    ```ts
    wcli.text("hoge").set("hello");
    ```

- <b class="tab-title">Python</b>
    ```python
    wcli.text("hoge").set("hello")
    ```

</div>

\note
<span class="since-c">1.10</span>
<span class="since-js">1.6</span>
Textの内部データは文字列だけでなく数値やbool値も型を保持して扱えるようになっています。
([View](./13_view.md)のInputRefで内部的に使用するため)  
C++はset,getで文字列と同様に送信、受信できます。  
JavaScriptではsetAny, getAny関数を使うと文字列以外のデータを処理できます。

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

Member::text() でTextクラスのオブジェクトが得られ、
Text::tryGet(), Text::tryGetRecurse() で値のリクエストをするとともに受信した値を取得できます。
(Dict はC++のみ)

例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    std::optional<std::string> hoge = wcli.member("foo").text("hoge").tryGet();
    ```
    初回の呼び出しではまだ受信していないため、
    tryGet() はstd::nulloptを返します。  
    get() はstd::nulloptの代わりにデフォルト値を返します。  
    また、std::string にキャストすることでも同様に値が得られます。

    <span class="since-c">2.0</span> ワイド文字列は tryGetW(), getW() で得られます。
    また、std::wstring にキャストすることでも得られます。

    std::ostreamにTextを直接渡して表示することもできます。
    ```cpp
    std::cout << "hoge = " << wcli.member("foo").text("hoge") << std::endl;
    ```

    \warning
    <span class="since-c">1.11</span>
    Textオブジェクト同士を比較するとTextが参照するデータの名前が一致するかどうかで判定されます。(Valueなど他のデータ型でも同様です。)  
    値を比較したい場合は明示的にキャストするか`get()`などを呼んでください。

- <b class="tab-title">C</b>
    \since <span class="since-c">1.7</span>

    wcfTextGet, (<span class="since-c">2.0</span> wcfTextGetW)
    に受信した文字列を格納するバッファとそのサイズ(null終端を含む)を指定します。
    ```c
    char text[6];
    int size;
    int ret = wcfTextGet(wcli, "a", "hoge", text, 6, &size);
    // ex.) ret = WCF_NOT_FOUND

    // few moments later,
    ret = wcfTextGet(wcli, "a", "hoge", text, 6, &size);
    // ex.) ret = WCF_OK, text = "hello\0", size = 5
    ```
    sizeに受信した文字列の長さ、バッファに受信した文字列が入ります。
    文字列がバッファの長さを超える場合は、(バッファのサイズ - 1) の文字列とnullが格納されます。

    初回の呼び出しでは`WCF_NOT_FOUND`を返し、別スレッドでリクエストが送信されます。

    \note member名に空文字列またはNULLを指定すると自分自身を指します。

- <b class="tab-title">JavaScript</b>
    ```ts
    const hoge: string | null = wcli.member("foo").text("hoge").tryGet();
    ```
    初回の呼び出しではまだ受信していないため、
    tryGet() はnullを返します。  
    get() はnullの代わりに空文字列を返します。
- <b class="tab-title">Python</b>
    ```python
    hoge = wcli.member("foo").text("hoge").try_get()
    ```
    初回の呼び出しではまだ受信していないため、
    try_get() はNoneを返します。  
    get() はNoneの代わりに空文字列を返します。

</div>

~~その後Client::sync()したときに実際にリクエストが送信され、~~  
<span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
別スレッドでリクエストが送信され、それ以降は値が得られるようになります。

\note
<span class="since-c">1.7</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
Text::request()で明示的にリクエストを送信することもできます。

### 時刻

~~Text::time()~~ でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。  
<span class="since-c">1.7</span>
<span class="since-js">1.6</span>
<span class="since-py"></span>
Member::syncTime() に変更

### Entry

~~Member::texts() で~~ そのMemberが送信しているtextのリストが得られます  
<span class="since-c">1.6</span>
<span class="since-py">1.1</span>
Member::textEntries() に変更

また、Member::onTextEntry() で新しくデータが追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Value](10_value.md) | [View](13_view.md) |

</div>
