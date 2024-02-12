# Text

API Reference →
C++ webcface::Text
JavaScript [Text](https://na-trium-144.github.io/webcface-js/classes/Text.html)
Python [webcface.Text](https://na-trium-144.github.io/webcface-python/webcface.text.html#webcface.text.Text)

文字列データを送受信します。

使い方は[Value](./10_value.md)とほぼ同じです。

## コマンドライン

```sh
webcface-send -t text
```
を実行し、文字列を入力すると送信されます。(1つ入力するごとに改行してください)

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

- <b class="tab-title">JavaScript</b>
    ```ts
    wcli.text("hoge").set("hello");
    ```

- <b class="tab-title">Python</b>
    ```python
    wcli.text("hoge").set("hello")
    ```

</div>


### 複数の値をまとめて送る

<div class="tabbed">

- <b class="tab-title">C++</b>
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
    tryGet(), tryGetRecurse() はstd::nulloptを返します。  
    get(), getRecurse() はstd::nulloptの代わりにデフォルト値を返します。  
    また、std::string, Text::Dict などの型にキャストすることでも同様に値が得られます。
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
    tryGet() はNoneを返します。  
    get() はNoneの代わりに空文字列を返します。

</div>

~~その後Client::sync()したときに実際にリクエストが送信され、~~  
![c++ ver1.2](https://img.shields.io/badge/1.2~-00599c?logo=C%2B%2B)
![js ver1.1](https://img.shields.io/badge/1.1~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
別スレッドでリクエストが送信され、それ以降は値が得られるようになります。

@note ![js ver1.1](https://img.shields.io/badge/1.1~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
Text::request()で明示的にリクエストを送信することもできます。

### 時刻

Text::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

@note Pythonでは Member.sync_time()

### Entry

Member::texts() でそのMemberが送信しているtextのリストが得られます

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    for(const webcface::Text &v: wcli.member("foo").texts()){
        // ...
    }
    ```
- <b class="tab-title">JavaScript</b>
    ```js
    for(const v of wcli.member("foo").texts()){
        // ...
    }
    ```
- <b class="tab-title">Python</b>
    ```python
    for v in wcli.member("foo").texts():
        # ...
    ```

</div>

Member::onTextEntry() で新しくデータが追加されたときのコールバックを設定できます

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.member("foo").onTextEntry().appendListener([](webcface::Text v){ /* ... */ });
    ```
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Text } from "webcface";
    wcli.member("foo").onTextEntry.on((v: Text) => { /* ... */ });
    ```
- <b class="tab-title">Python</b>
    ```python
    def text_entry(v: webcface.Text):
        pass
    wcli.member("foo").on_text_entry.connect(text_entry)
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
    wcli.member("foo").text("hoge").appendListener([](webcface::Text v){ /* ... */ });
    wcli.member("foo").onSync().appendListener([](webcface::Member m){ /* ... */ });
    ```
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Member, Text } from "webcface";
    wcli.member("foo").text("hoge").on((v: Text) => { /* ... */ });
    wcli.member("foo").onSync.on((m: Member) => { /* ... */ });
    ```
- <b class="tab-title">Python</b>
    pythonでは Text.signal プロパティがこのイベントのsignalを返します。
    ```python
    def text_change(v: webcface.Text):
        pass
    wcli.member("foo").text("hoge").signal.connect(text_change)
    def synced(m: webcface.Member):
        pass
    wcli.member("foo").on_sync.connect(synced)
    ```

</div>

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Value](10_value.md) | [View](13_view.md) |

</div>
