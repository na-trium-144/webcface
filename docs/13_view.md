# View

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

- <b class="tab-title">C</b>
    \since <span class="since-c">1.7</span>

    wcfViewComponent の配列を wcfViewSet に指定することで送信されます。

    例
    ```cpp
    wcfViewComponent vc[10];
    vc[0] = wcfText("hello world\n");
    char buf[10];
    sprintf(buf, "%d", i); // i はintの変数とか
    vc[1] = wcfText(buf);
    vc[2] = wcfNewLine(); // wcfText("\n") と同じ
    wcfFuncListen(wcli, "hoge", ...) // 関数の登録: 詳細は Func のページを参照
    vc[3] = wcfButton("a", NULL, "hoge");
    
    wcfViewSet(wcli, "a", vc, 4);
    wcli.sync();
    ```

    ![tutorial_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

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
    <span class="since-c">1.9</span> `webcface::Components` 名前空間に定義されています。
    ```cpp
    using namespace webcface::Components;
    ```
    をすると便利かもしれません
    \note namespace Components はinlineなので、 `webcface::` の名前空間でもアクセス可能です

    ver1.8以前はViewComponentsという名前空間でしたが現在はエイリアスになっておりどちらでもok

    各要素はそれぞれの関数から webcface::ViewComponent または webcface::TemporalComponent のオブジェクトとして得られます。
    `button(...).textColor(...)` などのようにメソッドチェーンすることで各要素にオプションを設定できます。


- <b class="tab-title">JavaScript</b>
    JavaScriptでは [`viewComponents`](https://na-trium-144.github.io/webcface-js/variables/viewComponents.html) オブジェクト内にそれぞれの要素を表す関数があります
    ```ts
    import { viewComponents } from "webcface";
    ```
    オプションはそれぞれ関数の引数にオブジェクトで渡すことができます。(詳細はこの後のそれぞれの要素の説明を参照)

- <b class="tab-title">Python</b>
    Pythonでは [`webcface.view_components`](https://na-trium-144.github.io/webcface-python/webcface.view_components.html) モジュール内にそれぞれの要素を表す関数があります
    ```python
    from webcface.view_components import *
    ```
    とすることもできます

    それぞれ関数のキーワード引数でオプションを設定できます。

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
    `text(文字列)`とし、さらにtextColorを指定することでテキストの色を変更することができます。
    ```cpp
    v.add(webcface::text("hello").textColor(webcface::ViewColor::red));
    v << webcface::text("hello").textColor(webcface::ViewColor::red);
    ```

- <b class="tab-title">C</b>
    wcfText でテキストを指定します。
    text_color でテキストの色を変更することができます。
    ```c
    vc[0] = wcfText("hello");
    vc[0].text_color = WCF_COLOR_RED;
    ```

- <b class="tab-title">JavaScript</b>
    string, number, boolean は文字列に変換されます。
    ```ts
    wcli.view("hoge").set([
        "hello",
        123,
    ]);
    ```
    `text(文字列)`を使ってtextColorを指定するとテキストの色を変更することができます。
    ```ts
    import { viewComponents, viewColor } from "webcface";
    wcli.view("hoge").set([
        viewComponents.text("hello", { textColor: viewColor.red }),
        123,
    ]);
    ```

- <b class="tab-title">Python</b>
    str, int, float, bool はaddの引数に直接指定すると文字列に変換されます。
    ```cpp
    v.add("hello").add(123)
    ```
    `text(文字列)`を使ってtext_colorを指定するとテキストの色を変更することができます。
    ```cpp
    v.add(webcface.view_components.text("hello", text_color=webcface.view_components.view_color.RED))
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
    wcfText に`\n`を渡すか、 wcfNewLine で指定できます。
    wcfTextの文字列の途中に`\n`がある場合もそこで改行されます
    ```c
    vc[0] = wcfText("\n");
    vc[0] = wcfNewLine();
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
    v << webcface::button("表示する文字列", wcli.func("hoge"));
    // v.add(...) でも同様
    ```
    関数を直接渡す場合
    (非表示のFuncとして登録され、他のFuncと同様に扱われます。
    WebUIや他MemberからはFuncの存在は見えません)
    ```cpp
    v << webcface::button("表示する文字列", [](){ /* ... */ });
    ```
    <span class="since-c">1.6</span> AnonymousFunc  
    関数を直接渡す場合と同様Funcの存在は見えません  
    Funcオブジェクトと同様実行条件などのオプションを設定することができます。
    ```cpp
    v << webcface::button(
        "表示する文字列",
        wcli.func([](){ /* ... */ })/*.setRunCond...*/
    );
    ```
    文字の色、背景色を設定できます
    (デフォルトではどちらも `ViewColor::inherit` で、その場合WebUI上では文字色=black、背景色=greenになります)
    ```cpp
    v << webcface::button(/* ... */)
            .textColor(webcface::ViewColor::red)
            .bgColor(webcface::ViewColor::yellow);
    ```

- <b class="tab-title">C</b>
    関数の登録方法は [Func](./30_func.md) を参照してください。
    表示する文字列に加え登録したFuncのmember名と名前をwcfButtonに指定します。
    member名をNULLまたは空文字列にすると自分自身が登録した関数を指します。
    ```c
    vc[0] = wcfButton("表示する文字列", NULL, "hoge");
    ```
    text_color, bg_color でテキストと背景の色を変更することができます。
    ```c
    vc[0].text_color = WCF_COLOR_RED;
    vc[0].bg_color = WCF_COLOR_YELLOW;
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

\warning
次の例のようにview内にbuttonが出現したり消滅したりする実装は非推奨です。
その切り替わりのタイミングでbuttonから呼び出される関数の設定が前後のbuttonとずれる場合があります。
```cpp
while (true){
    auto v = wcli.view("a");
    if(some_condition){
        // some_conditionによって、button1が表示されたりされなかったりする
        v << webcface::button("button1", ...);
    }
    v << webcface::button("button2", ...);
    v.sync();
    wcli.sync();
}
```
この後説明するinput要素についても同様です。  
インタラクティブな動作を伴わないtextやnewLineに関しては追加・削除しても問題ありません。

### input
\since <span class="since-c">1.10</span><span class="since-js">1.6</span>

viewに入力欄を表示します。

- textInput: 文字列入力
- decimalInput: 小数入力
- numberInput: 整数の入力
- selectInput: リストから値を選択させる
- toggleInput: クリックするたびに値が切り替わる
- sliderInput: 数値を指定するスライダー
- checkInput: チェックボックス

<div class="tabbed">

- <b class="tab-title">C++</b>
    InputRef  
    入力された値にアクセスするため webcface::InputRef オブジェクトを作成し、inputにbindします。
    そのInputRefオブジェクトをコピーまたは参照で別の関数などに渡すと、あとから値を取得することができます。
    ```cpp
    static webcface::InputRef input_val;
    v << webcface::button("cout ",
                          [=] { std::cout << input_val << std::endl; })
      << webcface::textInput("表示する文字列").bind(input_val)
      << std::endl;
    ```

    \warning
    上の例ではinput_valをstatic変数にし寿命が切れないようにしていますが、
    次の例のようにviewの生成ごとにInputRefオブジェクトを生成・破棄しても動作はします。
    ```cpp
    while (true){
        auto v = wcli.view("a");
        webcface::InputRef input_val;
        v << webcface::button("cout ",
                              [=] { std::cout << input_val << std::endl; })
          << webcface::textInput("表示する文字列").bind(input_val)
          << std::endl;
        // std::cout << "input_val = " << input_val.get(); この場合ここでは使えない
        v.sync();
        wcli.sync();
    }
    ```
    この場合はv.sync()の時に前周期のinput_valの内容が復元されるという挙動になります。
    (したがってv.sync()より前では値が未初期化になります)

    \note
    内部の実装では入力値を受け取りInputRefに値をセットする関数をonChangeにセットしています。
    また、InputRefの値は[Text](./11_text.md)の1つとしてviewを表示しているクライアントに送信されます。

    onChange  
    onChange() で値が入力されたときに実行する関数を設定でき、こちらでも値が取得できます。
    buttonに渡す関数と同様、関数オブジェクト、Funcオブジェクト、AnonymousFuncオブジェクトが使用できます。
    ```cpp
    v << webcface::textInput("表示する文字列").onChange([](std::string val) {
        std::cout << "input changed: " << val << std::endl;
    });
    ```

    その他各種inputに指定できるオプションには以下のものがあります。
    ([Func](./30_func.md)のArgオプションと同様です。)

    `.init(初期値)`  
    `.min(最小値)`, `.max(最大値)`: decimalInput, numberInput, sliderInputのみ  
    `.min(最小文字数)`, `.max(最大文字数)`: textInputのみ  
    `.step(刻み幅)`: numberInput, sliderInputのみ  
    `.option({ 選択肢, ... })`: selectInput, toggleInput  

- <b class="tab-title">JavaScript</b>
    InputRef  
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
    また、InputRefの値は[Text](./11_text.md)の1つとしてviewを表示しているクライアントに送信されます。

    onChange  
    onChange で値が入力されたときに実行する関数を設定でき、こちらでも値が取得できます。
    buttonに渡す関数と同様、関数オブジェクト、Funcオブジェクト、AnonymousFuncオブジェクトが使用できます。
    ```ts
    viewComponents.textInput("表示する文字列", {
        onChange: (val: string | number | boolean) => console.log(val),
    })
    ```

    その他各種inputに指定できるオプションには以下のものがあります。
    ([Func](./30_func.md)のArgオプションと同様です。)

    `init: 初期値`  
    `min: 最小値, max: 最大値`: decimalInput, numberInput, sliderInputのみ  
    `min: 最小文字数, max: 最大文字数`: textInputのみ  
    `step: 刻み幅`: numberInput, sliderInputのみ  
    `option: [選択肢, ... ]`: selectInput, toggleInput  


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

<span class="since-c">1.7</span>
Cの場合は wcfViewGet で wcfViewComponent の配列が得られます。
取得した配列は不要になったら wcfDestroy で破棄してください。

### onClick

ViewComponent::onClick() でボタン要素のクリック時に実行するべき関数が[Func](./30_func.md)オブジェクトとして取得できます。
したがって、ボタンを表示し、クリックされたときに`onClick().runAsync()`などとすることでそのボタンを動作させられます。

### onChangeとbind
<span class="since-c">1.10</span>
<span class="since-js">1.6</span>

各種Input要素の現在の値は ViewComponent::bind() で[Text](./11_text.md)オブジェクトとして取得できます。
したがって`bind()`の値をInputの初期値として使用すればよいです。
bind().get()で得られる webcface::ValAdaptor の値を整数、実数、bool、stringにキャストすることができます。

Inputの値を変更する際は、(view送信側がbindを設定したかonChangeを設定したかに関わらず)
ViewComponent::onChange() を使います。
引数に変化後の値を渡して`onChange().runAsync("変化後の値")`などとすることで
onChangeに設定された関数を実行すると同時にbindの値も変更されます。

### id
<span class="since-c">1.10</span>
<span class="since-js">1.6</span>

ViewComponent::id() で各要素に割り振られたid(文字列)を取得できます。
このidはそのview内で一意で、(buttonやInputの総数や順序が変わらなければ)
同じbutton、同じinputには常に同じidが振られます。
(実際はそのview内で種類ごとに分けて要素に連番を振っているだけです)

### 時刻

~~View::time()~~ でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。  
<span class="since-c">1.7</span>
<span class="since-js">1.6</span>
<span class="since-py"></span>
Member::syncTime() に変更

### Entry

~~Member::views() で~~ そのMemberが送信しているviewのリストが得られます  
<span class="since-c">1.6</span>
<span class="since-py">1.1</span>
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
