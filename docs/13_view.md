# View

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::View
* JavaScript [View](https://na-trium-144.github.io/webcface-js/classes/View.html)
* Python [webcface.View](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.View)

テキストやボタンなどの配置を送受信します。

## 送信

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::view からViewオブジェクトを作り、
    View::add() などで要素を追加し、
    最後にView::sync()をしてからClient::sync()をすることで送信されます。

    Viewはstd::ostreamを継承しており、 add() の代わりに v << 表示する値; というようにもできます。
    ostreamに出力可能なものはそのままviewにテキストとして出力できます。
    ostreamと同様にフォーマットを指定したり、std::endlで改行もできます。

    例
    ```cpp
    webcface::View v = wcli.view("a");
    // v.init(); // ←オブジェクトvを新規に構築せず繰り返し使いまわす場合は必要
    v << "hello world" << std::endl; // v.add("hello world").add("\n") と等価
    v << i << std::endl; // i はintの変数とか
    v << webcface::button("a", [] { std::cout << "hello" << std::endl; }) << std::endl;
    v.sync(); // ここまでにvに追加したものをクライアントに反映
    wcli.sync();
    ```
    ![tutorial_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

    C++ではViewのデストラクタでも自動的にView.sync()が呼ばれます。
    ```cpp
    {
        webcface::View v = wcli.view("a");
        v << ...;
        v << ...;
        // v.sync(); (自動で呼ばれる)
    }
    wcli.sync();
    ```
    \note
    <span class="since-c">1.2</span>
    Viewオブジェクトをコピーした場合、Viewオブジェクトの内容はコピーされるのではなく共有され、そのすべてのコピーが破棄されるまでsync()は呼ばれません。

- <b class="tab-title">JavaScript</b>
    Client::view からViewオブジェクトを作り、
    set()の引数に要素をまとめてセットして使います。

    例
    ```ts
    wcli.view("a").set([
        "hello world\n",
        i, // i は適当な変数とか
        "\n",
        viewComponents.button("a", () => console.log("hello"))
    ]);
    ```
    ![tutorial_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

- <b class="tab-title">Python</b>
    Client.view からViewオブジェクトを作り、
    View.add() などで要素を追加し、
    最後にView.sync()をしてからClient.sync()をすることで送信されます。
    
    例
    ```py
    v = wcli.view("a")
    # v.init() ←オブジェクトvを新規に構築せず繰り返し使いまわす場合は必要
    v.add("hello world\n")
    v.add(i).add("\n") # i は適当な変数とか
    v.add(webcface.view_components.button("a", lambda: print("hello")))
    v.sync()
    ```
    ![tutorial_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

    with構文を使って `with wcli.view("hoge") as v:` などとするとwithを抜けるときに自動でv.sync()がされます。
    ```py
    with wcli.view("a") as v:
        v.add(...)
        v.add(...)
        # v.sync() (自動で呼ばれる)
    ```

</div>

\note
Viewの2回目以降の送信時にはWebCFace内部では前回からの差分のみが送信されるので、通信量が削減されます

## ViewComponent
Viewに追加する各種要素をViewComponentといいます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    C++では `webcface::ViewComponents` 名前空間に定義されています。
    ```cpp
    using namespace webcface::ViewComponents;
    ```
    をすると便利かもしれません
    \note ViewComponentsはinlineなので、 `webcface::` の名前空間でもアクセス可能です

- <b class="tab-title">JavaScript</b>
    JavaScriptでは [`viewComponents`](https://na-trium-144.github.io/webcface-js/variables/viewComponents.html) オブジェクト内にあります
    ```ts
    import { viewComponents } from "webcface";
    ```

- <b class="tab-title">Python</b>
    Pythonでは [`webcface.view_components`](https://na-trium-144.github.io/webcface-python/webcface.view_components.html) モジュール内にあり、
    ```python
    from webcface.view_components import *
    ```
    とすることもできます

</div>

### text
文字列です。そのまま表示します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    std::ostreamでフォーマット可能なデータはそのまま渡して文字列化できます。
    View::add()関数, set()関数でも同様に文字列に変換されます。
    ```cpp
    v.add("hello").add(123);
    v << "hello" << 123;
    ```
    `text(文字列)`を使うとテキストの色を変更することができます。
    ```cpp
    v.add(webcface::ViewComponents::text("hello").textColor(webcface::ViewColor::red));
    v << webcface::ViewComponents::text("hello").textColor(webcface::ViewColor::red);
    ```

- <b class="tab-title">JavaScript</b>
    string, number, boolean は文字列に変換されます。
    ```ts
    wcli.view("hoge").set([
        "hello",
        123,
    ]);
    ```
    `text(文字列)`を使うとテキストの色を変更することができます。
    ```ts
    import { viewComponents, viewColor } from "webcface";
    wcli.view("hoge").set([
        viewComponents.text("hello", { textColor: viewColor.red }),
        123,
    ]);
    ```

- <b class="tab-title">Python</b>
    str, int, float, bool は文字列に変換されます。
    ```cpp
    v.add("hello").add(123)
    ```
    `text(文字列)`を使うとテキストの色を変更することができます。
    ```cpp
    v.add(webcface.view_components.text("hello", text_color=webcface.view_components.view_color.RED))
    ```

</div>

### newLine
改行します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    `webcface::ViewComponents::newLine()` の他、`std::endl`や`"\n"`でも改行できます。
    `\n`は単体でなく文字列中にあってもそこで改行されます。
    ```cpp
    v.add("hello\nhello");
    v.add("hello").add("\n").add("hello");
    v << "hello\nhello";
    v << "hello" << std::endl << "hello";
    using namespace webcface::ViewComponents;
    v.add(text("hello")).add(newLine()).add(text("hello"));
    v << text("hello") << newLine() << text("hello");
    ```
- <b class="tab-title">JavaScript</b>
    `newLine()`の他`"\n"`でも改行できます。
    `\n`は単体でなく文字列中にあってもそこで改行されます。
    ```ts
    wcli.view("hoge").set([
        "hello\nhello",
        viewComponents.newLine(),
        "hello",
    ]);
    ```

- <b class="tab-title">Python</b>
    `new_line()` の他、`"\n"`でも改行できます。
    `\n`は単体でなく文字列中にあってもそこで改行されます。
    ```cpp
    v.add("hello\nhello");
    v.add("hello").add("\n").add("hello");
    v.add(text("hello")).add(new_line()).add(text("hello"));
    ```

</div>


### button
ボタンを表示します。

クリック時の動作は、関数を登録済みの[Funcオブジェクト](./30_func.md)、または関数を直接設定できます。

\note 別のMemberのFuncオブジェクトを渡すこともできます
(ボタンを押すと別のMemberに登録されている関数が実行される)

<div class="tabbed">

- <b class="tab-title">C++</b>
    Funcオブジェクトの場合
    ```cpp
    wcli.func("hoge").set(/*...*/);
    v << webcface::ViewComponents.button("表示する文字列", wcli.func("hoge"));
    // v.add(...) でも同様
    ```
    関数を直接渡す場合
    (WebUIや他MemberからはFuncの存在は見えません)
    ```cpp
    v << webcface::ViewComponents.button("表示する文字列", [](){ /* ... */ });
    ```
    <span class="since-c">1.6</span> AnonymousFunc  
    関数を直接渡す場合と同様Funcの存在は見えません  
    Funcオブジェクトと同様実行条件などのオプションを設定することができます。
    ```cpp
    v << webcface::ViewComponents.button(
        "表示する文字列",
        wcli.func([](){ /* ... */ })/*.setRunCond...*/
    );
    ```
    文字の色、背景色を設定できます
    (デフォルトではどちらも `ViewColor::inherit` で、その場合WebUI上では文字色=black、背景色=greenになります)
    ```cpp
    v << webcface::ViewComponents::button(/* ... */)
            .textColor(webcface::ViewColor::red)
            .bgColor(webcface::ViewColor::yellow);
    ```
- <b class="tab-title">JavaScript</b>
    Funcオブジェクトの場合
    ```ts
    wcli.func("hoge").set(/* ... */);
    wcli.view("hoge").set([
        viewComponents.button("表示する文字列", wcli.func("hoge")),
    ]);
    ```
    関数を直接渡す場合
    (WebUIや他MemberからはFuncの存在は見えません)
    ```ts
    wcli.view("hoge").set([
        viewComponents.button("表示する文字列", () => {/* ... */})
    ]);
    ```
    文字の色、背景色を設定できます
    (デフォルトではどちらも `viewColor.inherit` で、その場合WebUI上では文字色=black、背景色=greenになります)
    ```ts
    wcli.view("hoge").set([
        viewComponents.button(/* ... */, {
            textColor: viewColor.red,
            bgColor: viewColor.yellow,
        })
    ]);
    ```
- <b class="tab-title">Python</b>
    Funcオブジェクトの場合
    ```py
    wcli.func("hoge").set(...)
    v.add(webcface.view_components.button("表示する文字列", wcli.func("hoge")))
    ```
    関数を直接渡す場合
    (WebUIや他MemberからはFuncの存在は見えません)
    ```py
    def hoge():
        pass
    v.add(webcface.view_components.button("表示する文字列", hoge));
    ```
    文字の色、背景色を設定できます
    (デフォルトではどちらも `view_color.INHERIT` で、その場合WebUI上では文字色=black、背景色=greenになります)
    ```py
    v.add(webcface.view_components.button(
        ... ,
        text_color=webcface.view_components.view_color.RED,
        bg_color=webcface.view_components.view_color.YELLOW,
    ))
    ```

</div>

## 受信

ValueやTextと同様、Member::view() でViewクラスのオブジェクトが得られ、
View::tryGet(), View::get() で受信したViewデータを取得できます。
(これを使うのはViewを表示するアプリを作る場合などですかね)

Viewデータは
webcface::ViewComponent
(JavaScript [ViewComponent](https://na-trium-144.github.io/webcface-js/classes/ViewComponent.html),
Python [webcface.ViewComponent](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.ViewComponent))
のリストとして得られ、
ViewComponentオブジェクトから各種プロパティを取得できます。
onClick()で得られるFuncオブジェクトは`runAsync()`などでそのまま実行させることができます。


### 時刻

View::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

\note Pythonでは Member.sync_time()

### Entry

~~Member::views() で~~ そのMemberが送信しているviewのリストが得られます  
<span class="since-c">1.6</span>
Member::viewEntries() に変更

また、Member::onViewEntry() で新しくデータが追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Text](11_text.md) | [Canvas2D](14_canvas2d.md) |

</div>
