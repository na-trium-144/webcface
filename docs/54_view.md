# 5-4. View

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::View (`webcface/view.h`)
* C Reference: c_wcf/view.h
* JavaScript [View](https://na-trium-144.github.io/webcface-js/classes/View.html)
* Python [webcface.View](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.View)

テキストやボタンなどの配置を送受信します。

## 送信

\note
Viewの2回目以降の送信時にはWebCFace内部では前回からの差分のみが送信されるので、通信量が削減されます

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::view からViewオブジェクトを作り、
    View::add() などで要素を追加し、
    最後にView::sync()をしてからClient::sync()をすることで送信されます。

    Viewはstd::ostreamを継承しており、 add() の代わりに `v << 表示する値;` というようにもできます。
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
    ![example_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_view.png)

    <span class="since-c">2.6</span>
    View::inserter() が返すイテレーターを使って、 `fmt::format_to()` や `std::format_to()` で出力することもできます。
    ```cpp
    #include <fmt/base.h>

    fmt::format_to(v.inserter(), "with inserter: {}\n", i);
    ```

    \warning
    <span class="since-c">2.0</span>
    ワイド文字列を出力したい場合はostreamやinserterに直接渡すのではなく Component::text を使う必要があります。
    (後述)

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


- <b class="tab-title">C</b>
    \since <span class="since-c">1.7</span>

    wcfViewComponent, (<span class="since-c">2.0</span> wcfViewComponentW)
    の配列を wcfViewSet, (<span class="since-c">2.0</span> wcfViewSetW) に指定することで送信されます。

    例
    ```c
    wcfViewComponent vc[10];
    vc[0] = wcfText("hello world\n");
    char buf[10];
    sprintf(buf, "%d", i); // i はintの変数とか
    vc[1] = wcfText(buf);
    vc[2] = wcfNewLine(); // wcfText("\n") と同じ
    wcfFuncListen(wcli, "hoge", ...) // 関数の登録: 詳細は Func のページを参照
    vc[3] = wcfButton("a", NULL, "hoge");
    
    wcfViewSet(wcli, "a", vc, 4);
    wcfSync();
    ```

    ![example_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_view.png)

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
    ![example_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_view.png)

- <b class="tab-title">Python</b>
    Client.view からViewオブジェクトを作り、
    View.add() などで要素を追加し、
    最後にView.sync()をしてからClient.sync()をすることで送信されます。
    
    例
    ```py
    v = wcli.view("a")
    # v.init() ←オブジェクトvを新規に構築せず繰り返し使いまわす場合は必要
    v.add("hello world\n")
    v.add(i, "\n") # i は適当な変数とか
    v.add(webcface.components.button("a", lambda: print("hello")))
    v.sync()
    ```
    add() にはコンマ区切りで複数の要素を渡すこともできます(1つずつadd()するのと同じです)
    
    ![example_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_view.png)

    with構文を使って `with wcli.view("hoge") as v:` などとするとwithを抜けるときに自動でv.sync()がされます。
    ```py
    with wcli.view("a") as v:
        v.add(...)
        v.add(...)
        # v.sync() (自動で呼ばれる)
    ```

</div>

## ViewComponent
Viewに追加する各種要素をViewComponentといいます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <del>`webcface::ViewComponents` </del>  
    <span class="since-c">1.9</span> <del>`webcface::Components`</del>  
    <span class="since-c">2.0</span> `webcface::components`
    名前空間に定義されています。

    ```cpp
    using namespace webcface::components;
    ```
    をすると便利かもしれません

    \note
    * namespace components はinlineなので、 `webcface::` の名前空間でもアクセス可能です。
    * また以前のnamespace名もエイリアスになっておりどちらでもokです。

    各要素はそれぞれの関数から webcface::TemporalViewComponent または webcface::TemporalComponent のオブジェクトとして得られます。

    <span class="since-c">1.11</span>
    引数にView(コピーまたはconst参照)を取る関数オブジェクトをViewに渡すと、その場でその関数が呼び出されます。
    複数のViewComponentを出力する処理をまとめて使いまわしたい場合に便利です。
    ```cpp
    auto showNameAndValue(const std::string &name, int value) {
        return [=](const webcface::View &view) {
            view << name << " = " << value;
        };
    }

    webcface::View v = wcli.view("a");
    v << showNameAndValue("foo", 123) << std::endl; // v << "foo = 123";
    v.sync();
    ```

- <b class="tab-title">JavaScript</b>
    JavaScriptでは [`viewComponents`](https://na-trium-144.github.io/webcface-js/variables/viewComponents.html) オブジェクト内にそれぞれの要素を表す関数があります
    ```ts
    import { viewComponents } from "webcface";
    ```

- <b class="tab-title">Python</b>
    Pythonでは <del>`webcface.view_components`</del>
    <span class="since-c">3.0</span> [`webcface.components`](https://na-trium-144.github.io/webcface-python/webcface.components.html) モジュール内にそれぞれの要素を表す関数があります
    ```python
    from webcface.components import *
    ```
    とすることもできます

</div>

### text
文字列です。そのまま表示します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Viewをostreamとして使うことで、std::ostreamでフォーマット可能なデータはそのまま渡して文字列化できます。

    View::add() 関数に数値や文字列などを渡すことでも文字列に変換されます。
    ```cpp
    v.add("hello").add(123);
    ```
    
    文字列を直接渡す代わりに `text(文字列)` でViewComponentに変換すると、textColorなど後述のオプションを指定することもできるようになります。
    ```cpp
    v << webcface::text("hello").textColor(webcface::ViewColor::red);
    ```

    <span class="since-c">2.0</span>
    Viewに直接ワイド文字列を出力することはできませんが、text()の引数にはワイド文字列も使用可能です。

    <span class="since-c">2.6</span>
    View::inserter() が返すイテレーターを使って、 `fmt::format_to()` や `std::format_to()` で出力することもできます。

- <b class="tab-title">C</b>
    wcfText, (<span class="since-c">2.0</span> wcfTextW) でテキストを指定します。
    ```c
    vc[0] = wcfText("hello");
    ```

- <b class="tab-title">JavaScript</b>
    string, number, boolean は文字列に変換されます。
    ```ts
    wcli.view("hoge").set([
        "hello",
        123,
    ]);
    ```
    文字列を直接渡す代わりに `text(文字列)` でViewComponentに変換すると、textColorなど後述のオプションを指定することもできるようになります。
    ```ts
    import { viewComponents, viewColor } from "webcface";
    wcli.view("hoge").set([
        viewComponents.text("hello", { textColor: viewColor.red }),
        123,
    ]);
    ```

- <b class="tab-title">Python</b>
    str, int, float, bool はaddの引数に直接指定すると文字列に変換されます。
    ```python
    v.add("hello").add(123)
    ```
    文字列を直接渡す代わりに `text(文字列)` でViewComponentに変換すると、text_colorなど後述のオプションを指定することもできるようになります。
    ```python
    v.add(components.text("hello", text_color=webcface.ViewColor.RED))
    ```
    
    <span class="since-c">3.0</span>
    オプションはadd()の引数としても渡すことができます。
    ```python
    v.add("hello", text_color=webcface.ViewColor.RED)
    ```
    

</div>

### newLine
改行します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    `webcface::newLine()` の他、`std::endl`や`"\n"`でも改行できます。
    `\n`は単体でなく文字列中にあってもそこで改行されます。
    ```cpp
    v.add("hello\nhello");
    v.add("hello").add("\n").add("hello");
    v << "hello\nhello";
    v << "hello" << std::endl << "hello";
    using namespace webcface::Components;
    v.add(text("hello")).add(newLine()).add(text("hello"));
    v << text("hello") << newLine() << text("hello");
    ```
- <b class="tab-title">C</b>
    wcfText に`\n`を渡すか、 wcfNewLine, (<span class="since-c">2.0</span> wcfNewLineW) で指定できます。
    wcfTextの文字列の途中に`\n`がある場合もそこで改行されます
    ```c
    vc[0] = wcfText("\n");
    vc[0] = wcfNewLine();
    ```

    \warning
    newLineには文字列を渡す機能はないですが、他の要素と型を合わせるためにwcfNewLineとwcfNewLineWを使い分ける必要があります

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

クリック時の動作は、関数を登録済みの[Funcオブジェクト](./53_func.md)かFuncListenerオブジェクトを指定するか、または関数を直接設定できます。

\note 別のMemberのFuncオブジェクトを渡すこともできます
(ボタンを押すと別のMemberに登録されている関数が実行される)

\warning
<span class="since-c">2.5</span>
<span class="since-py">3.0</span>
buttonや、このあと説明するinputなど、インタラクティブな動作を伴う要素には id として一意な文字列を指定してください。
通常は指定しなくても動作しますが、
次の例のようにview内に要素が出現したり消滅したりするようなものを書いた場合
その切り替わりのタイミングで要素の内容がずれる場合があります。
```cpp
auto v = wcli.view("a");
if(some_condition){
    // some_conditionによって表示されたりされなかったりする
    v << webcface::button(...);
}
v << webcface::button(...);
v.sync();
```

<div class="tabbed">

- <b class="tab-title">C++</b>
    Funcオブジェクト、 <span class="since-c">2.5</span> FuncListerオブジェクト の場合
    ```cpp
    wcli.func("hoge").set(/*...*/);
    v << webcface::button("表示する文字列", wcli.func("hoge")).id("button1");
    // v.add(...) でも同様
    ```
    
    関数を直接渡す場合
    (非表示のFuncとして登録され、他のFuncと同様に扱われます。
    WebUIや他MemberからはFuncの存在は見えません)
    ```cpp
    v << webcface::button("表示する文字列", [](){ /* ... */ }).id("button1");
    ```

- <b class="tab-title">C</b>
    関数の登録方法は [Func](./53_func.md) を参照してください。
    表示する文字列に加え登録したFuncのmember名と名前を
    wcfButton, (<span class="since-c">2.0</span> wcfButtonW) に指定します。
    member名をNULLまたは空文字列にすると自分自身が登録した関数を指します。
    ```c
    vc[0] = wcfButton("表示する文字列", NULL, "hoge");
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

- <b class="tab-title">Python</b>
    Funcオブジェクト、FuncListenerオブジェクトの場合
    ```py
    wcli.func("hoge").set(...)
    v.add(components.button("表示する文字列", wcli.func("hoge"), id="button1"))
    ```
    関数を直接渡す場合
    (WebUIや他MemberからはFuncの存在は見えません)
    ```py
    def hoge():
        pass
    v.add(components.button("表示する文字列", hoge, id="button1"));
    ```

</div>

### input
\since <span class="since-c">1.10</span><span class="since-js">1.6</span><span class="since-py">2.0</span>

viewに入力欄を表示します。

- textInput: 文字列入力
  - <span class="since-c">2.6</span><span class="since-js">1.10</span><span class="since-py">3.1</span> height プロパティを2以上にすることで複数行入力欄になります。
- decimalInput: 小数入力
- numberInput: 整数の入力
- selectInput: リストから値を選択させる
- toggleInput: クリックするたびに値が切り替わる
- sliderInput: 数値を指定するスライダー
- checkInput: チェックボックス

#### InputRef

<div class="tabbed">

- <b class="tab-title">C++</b>
    入力された値にアクセスするため webcface::InputRef オブジェクトを作成し、inputにbindします。
    そのInputRefオブジェクトをコピーまたは参照で別の関数などに渡すと、あとから値を取得することができます。
    ```cpp
    static webcface::InputRef input_val;
    v << webcface::button("cout ",
                          [=] { std::cout << input_val << std::endl; })
        .id("button1")
      << webcface::textInput("表示する文字列").bind(input_val).id("input1")
      << std::endl;
    ```

    \warning
    上の例ではinput_valをstatic変数にし寿命が切れないようにしていますが、
    staticにできない場合(複数のviewで使い回す場合など)は次の例のようにviewの生成ごとにInputRefオブジェクトを生成・破棄しても動作はします。
    ```cpp
    while (true){
        auto v = wcli.view("a");
        webcface::InputRef input_val;
        v << webcface::button("cout ",
                              [=] { std::cout << input_val << std::endl; })
            .id("button1")
          << webcface::textInput("表示する文字列").bind(input_val).id("input1")
          << std::endl;
        // std::cout << "input_val = " << input_val.get(); この場合ここでは使えない
        v.sync();
        wcli.sync();
    }
    ```
    この場合はv.sync()の時に前周期のinput_valの内容が復元されるという挙動になります。
    (したがってv.sync()より前では値が未初期化になります)

    <span class="since-c">1.11</span>
    InputRefの値は
    `asStringRef()`, `asString()`, `asBool()`, <del>`as<double>()`</del> で型を指定して取得できます。  
    <span class="since-c">2.0</span> `asWStringRef()`, `asWString()`, `asDouble()`, `asInt()`, `asLLong()` も使えます。  
    (std::string, double, bool などの型にキャストすることでも値を得られます。)  
    (任意の型に対応したい場合は `get()` で webcface::ValAdaptor 型として取得できます。)

    \note
    内部の実装では入力値を受け取りInputRefに値をセットする関数をonChangeにセットしています。
    また、InputRefの値は[Text](./52_text.md)型のデータとしてviewを表示しているクライアントに送信されます。

    <span></span>

- <b class="tab-title">JavaScript</b>
    入力された値にアクセスするため [InputRef](https://na-trium-144.github.io/webcface-js/classes/InputRef.html) オブジェクトを作成し、inputにbindします。
    そのInputRefオブジェクトを別の関数などに渡すと、あとから値を取得することができます。
    ```ts
    import { InputRef, viewComponents } from "webcface";
    const inputVal = new InputRef();
    setInterval(() => {
        wcli.view("hoge").set([
            viewComponents.button("cout", () => console.log(inputVal.get())),
            viewComponents.textInput("表示する文字列", { bind: inputVal }),
        ]);
        wcli.sync();
    }, 100);
    ```
    \warning
    viewを繰り返し送信するときInputRefオブジェクトは同じものを使いまわすのでも、
    毎回新しいInputRefオブジェクトを生成するのでも、どちらでも動作します。
    ```cpp
    import { InputRef, viewComponents } from "webcface";
    setInterval(() => {
        const inputVal = new InputRef(); // 毎回新しいInputRef
        wcli.view("hoge").set([
            viewComponents.button("cout", () => console.log(inputVal.get())), // ok
            viewComponents.textInput("表示する文字列", { bind: inputVal }),
            // inputVal.get(), // ここでは使えない
        ]);
        wcli.sync();
    }, 100);
    ```
    この場合はview.set()が実行される時に前周期のinputValの内容が復元されるという挙動になります。
    (したがってv.set()の引数内ではinputValの値は未初期化になります)

    \note
    内部の実装では入力値を受け取りInputRefに値をセットする関数をonChangeにセットしています。
    また、InputRefの値は[Text](./52_text.md)型のデータとしてviewを表示しているクライアントに送信されます。

    <span></span>

- <b class="tab-title">Python</b>
    入力された値にアクセスするため InputRef オブジェクトを作成し、inputにbindします。
    そのInputRefオブジェクトをコピーまたは参照で別の関数などに渡すと、あとから値を取得することができます。
    ```python
    from webcface import InputRef
    input_val = InputRef()
    def print_val():
        print(str(input_val.get()))
    v.add(components.button("print", print_val, id="button1"))
    v.add(components.text_input("表示する文字列", bind=input_val, id="input1")
    ```

    \warning
    viewを繰り返し送信するときInputRefオブジェクトは同じものを使いまわすのでも、
    毎回新しいInputRefオブジェクトを生成するのでも、どちらでも動作します。
    ```python
    from webcface import InputRef

    while True:
        input_val = InputRef()
        def print_val():
            print(str(input_val.get()))
        with wcli.view("hoge") as v:
            v.add(components.button("print", print_val, id="button1"))
            v.add(components.text_input("表示する文字列", bind=input_val, id="input1")
            # print(input_val.get()) # ここでは使えない
            # v.sync()
        wcli.sync()
    ```
    この場合はview.sync()が実行される時に前周期のinput_valの内容が復元されるという挙動になります。
    (したがってv.sync()より前ではinput_valの値は未初期化になります)

    \note
    内部の実装では入力値を受け取りInputRefに値をセットする関数をonChangeにセットしています。
    また、InputRefの値は[Text](./52_text.md)型のデータとしてviewを表示しているクライアントに送信されます。

    <span></span>

</div>

#### onChange

<div class="tabbed">

- <b class="tab-title">C++</b>
    onChange() で値が入力されたときに実行する関数を設定でき、こちらでも値が取得できます。
    buttonに渡す関数と同様、関数オブジェクト、Funcオブジェクト、FuncListenerオブジェクトが使用できます。
    ```cpp
    v << webcface::textInput("表示する文字列").id("input1").onChange([](std::string val) {
        std::cout << "input changed: " << val << std::endl;
    });
    ```

    \note bindとonChangeを両方設定することはできません。

    <span></span>

- <b class="tab-title">JavaScript</b>
    onChange で値が入力されたときに実行する関数を設定でき、こちらでも値が取得できます。
    buttonに渡す関数と同様、関数、Funcオブジェクトが使用できます。
    ```ts
    viewComponents.textInput("表示する文字列", {
        onChange: (val: string | number | boolean) => console.log(val),
    })
    ```

    \note bindとonChangeを両方設定することはできません。

    <span></span>

- <b class="tab-title">Python</b>
    on_change で値が入力されたときに実行する関数を設定でき、こちらでも値が取得できます。
    buttonに渡す関数と同様、関数、ラムダ式、またはFuncオブジェクト、FuncListenerオブジェクトが使用できます。
    ```ts
    components.text_input("表示する文字列", id="input1", on_change=...)
    ```

    \note bindとonChangeを両方設定することはできません。

    <span></span>
    
</div>

### オプション

各ViewComponentには以下のオプションを指定することができます。
(要素の種類によっては効果がないものもあります)

* id: <span class="since-c">2.5</span><span class="since-py">3.0</span> インタラクティブな要素に指定するID
    * 指定する場合は一意な文字列を指定してください。
* textColor: 文字の色を変更します。
    * WebUIではデフォルトは黒です
* bgColor: 背景色を変更します。
    * WebUIではデフォルトは緑です
* width, height: <span class="since-c">2.6</span><span class="since-js">1.10</span><span class="since-py">3.1</span> 要素の幅、高さを指定します。
    * サイズの単位、0または負の場合の表示方法は実装依存です。
    * WebUIでは正の整数を指定すると 指定した値 \* 1em になります
    * デフォルトでは0になっています。
* 各種inputに指定できる入力データのオプション
([Func](./53_func.md)のArgオプションと同様です。)
    * init: 初期値
    * min: 最小値, max: 最大値 (decimalInput, numberInput, sliderInputのみ)
    * min: 最小文字数, max: 最大文字数 (textInputのみ)
    * step: 刻み幅 (numberInput, sliderInputのみ)
    * option: 選択肢 (selectInput, toggleInput)

\note
[webcface-tui](75_tui.md)では文字色と背景色に指定した白と黒がそれぞれ反転して表示されます。


<div class="tabbed">

- <b class="tab-title">C++</b>
    `button(...).textColor(...)` などのようにメソッドチェーンすることで各要素にオプションを設定できます。
    詳細は webcface::TemporalViewComponent のリファレンスを参照してください。

    色は webcface::ViewColor のenumで指定します。

- <b class="tab-title">C</b>
    wcfViewComponent 構造体のメンバーでオプションを指定することができます。

    ```c
    wcfViewComponent vc[10];
    vc[0] = wcfText("hello world\n");
    vc[0].text_color = WCF_COLOR_RED;
    ```

- <b class="tab-title">JavaScript</b>
    `button("text", { textColor: ... })`
    などのように、オプションはそれぞれ関数の引数にオブジェクトで渡すことができます。

- <b class="tab-title">Python</b>
    `button("text", text_color=...)` などのように、それぞれ関数のキーワード引数でオプションを設定できます。

    <span class="since-c">3.0</span>
    add() の引数にキーワード引数を追加しても同様に要素にオプションが追加されます。

</div>

## 受信

ViewデータはWebUIに表示するだけでなく、ValueやTextと同様プログラムから受信することもできます。
(これを使うのはViewを表示するアプリを作る場合などですかね)

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::view() でViewクラスのオブジェクトが得られ、
    View::tryGet(), View::get() で受信したViewデータを取得できます。

    Viewデータは
    webcface::ViewComponent
    のリストとして得られ、
    ViewComponentオブジェクトから各種プロパティを取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```cpp
    std::optional<std::vector<ViewComponent>> hoge = wcli.member("foo").view("hoge").tryGet();
    ```
    * 値をまだ受信していない場合 tryGet() はstd::nulloptを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient::sync()したときに</del>
        <span class="since-c">1.2</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * View::get() はstd::nulloptの代わりに空のvectorを返します。

    <span class="since-c">1.7</span>
    View::request() で明示的にリクエストを送信することもできます。

- <b class="tab-title">C</b>
    \since <span class="since-c">1.7</span>

    wcfViewGet, (<span class="since-c">2.0</span> wcfViewGetW) で
    wcfViewComponent, (<span class="since-c">2.0</span> wcfViewComponentW) の配列が得られます。
    
    取得した配列は不要になったら wcfDestroy で破棄してください。

- <b class="tab-title">JavaScript</b>
    Member.view() でViewクラスのオブジェクトが得られ、
    View.tryGet(), View.get() で受信したViewデータを取得できます。

    Viewデータは
    [ViewComponent](https://na-trium-144.github.io/webcface-js/classes/ViewComponent.html)
    のリストとして得られ、
    ViewComponentオブジェクトから各種プロパティを取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```ts
    const hoge: ViewComponent[] | null = wcli.member("foo").view("hoge").tryGet();
    ```
    * 値を受信していない場合 tryGet() はnullを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient.sync()したときに</del>
        <span class="since-js">1.1</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * get() はnullの代わりに空のリストを返します。

    <span class="since-js">1.1</span>
    View.request()で明示的にリクエストを送信することもできます。

- <b class="tab-title">Python</b>
    Member.view() でViewクラスのオブジェクトが得られ、
    View.tryGet(), View.get() で受信したViewデータを取得できます。

    Viewデータは
    [webcface.ViewComponent](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.ViewComponent)
    のリストとして得られ、
    ViewComponentオブジェクトから各種プロパティを取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```python
    hoge = wcli.member("foo").view("hoge").try_get()
    ```
    * 値を受信していない場合 try_get() はNoneを返し、そのデータのリクエストをサーバーに送ります。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度try_get()することで値が得られます。
    * get() はNoneの代わりに空のリストを返します。

    View.request()で明示的にリクエストを送信することもできます。

</div>

### onClick

ViewComponent::onClick() でボタン要素のクリック時に実行するべき関数が[Func](./53_func.md)オブジェクトとして取得できます。
したがって、ボタンを表示し、クリックされたときに`onClick().runAsync()`などとすることでそのボタンを動作させられます。

### onChangeとbind
<span class="since-c">1.10</span>
<span class="since-js">1.6</span>
<span class="since-py">2.0</span>

各種Input要素の現在の値は ViewComponent::bind() で
<del>[Text](./52_text.md)オブジェクトとして</del>
<span class="since-c">2.0</span> <span class="since-py">2.0</span> Variant オブジェクトとして取得できます。
したがって`bind()`の値をInputの初期値として使用すればよいです。

<!-- <span class="since-c">2.0</span> Variant の値は
`asStringRef()`, `asWStringRef()`, `asString()`, `asWString()`, `asBool()`, `asDouble()`, `asInt()`, `asLLong()` で型を指定して取得できます。  
(std::string, double, bool などの型にキャストすることでも値を得られます。)  
(bind().get() で webcface::ValAdaptor 型としても取得できます。)
 -->

Inputの値を変更する際は、(view送信側がbindを設定したかonChangeを設定したかに関わらず)
ViewComponent::onChange() を使います。
引数に変化後の値を渡して`onChange().runAsync("変化後の値")`などとすることで
onChangeに設定された関数を実行すると同時にbindの値も変更されます。

### id
<span class="since-c">1.10</span>
<span class="since-js">1.6</span>
<span class="since-py">3.0</span>

ViewComponent::id() で各要素に割り振られたid(文字列)を取得できます。
このidはそのview内で一意で、(buttonやInputの総数や順序が変わらなければ)
同じbutton、同じinputには常に同じidが振られます。

## Entry, イベントについて

[4-3. Field](43_field.md) に移動しました。そちらを参照してください


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [5-3. Func](53_func.md) | [5-5. Log](55_log.md) |

</div>
