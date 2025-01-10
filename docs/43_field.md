# 4-3. Field

\tableofcontents
\sa
* C++ webcface::Field (`webcface/field.h`)
* JavaScript [Field](https://na-trium-144.github.io/webcface-js/classes/Field.html)
* Python [webcface.Field](https://na-trium-144.github.io/webcface-python/webcface.field.html#webcface.field.Field)

WebCFaceで送受信されるそれぞれのデータを Field と呼びます。
データ型としては
[Value](51_value.md),
[Text](52_text.md),
[Func](53_func.md),
[View](54_view.md),
[Log](55_log.md),
[Canvas2D](61_canvas2d.md),
[Image](62_image.md),
[Canvas3D](63_canvas3d.md),
[RobotModel](64_robot_model.md)
が用意されており、詳細はそれぞれのページで説明しています。

このページには各データ型に共通の情報を載せています。

## 名前の指定

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">1.11</span>

    Client::child() または Member::child() に名前を指定することで Field オブジェクトを取得できます。

    ```cpp
    webcface::Field hoge = wcli.child("hoge"); // 自分の hoge という名前のデータ
    webcface::Field fuga = wcli.member("foo").child("fuga"); // foo というメンバーの fuga というデータ
    ```

    または `["名前"]` でも同様に取得できます。
    ```cpp
    webcface::Field hoge = wcli["hoge"];
    webcface::Field fuga = wcli.member("foo")["fuga"];
    // ↑ wcli["foo"]["fuga"] は別の意味になるので注意
    ```
    
- <b class="tab-title">JavaScript</b>
    \since

    Client.child() または Member.child() に名前を指定することで Field オブジェクトを取得できます。

    ```ts
    const hoge = wcli.child("hoge"); // 自分の hoge という名前のデータ
    const fuga = wcli.member("foo").child("fuga"); // foo というメンバーの fuga というデータ
    ```

- <b class="tab-title">Python</b>
    \since

    Client.child() または Member.child() に名前を指定することで Field オブジェクトを取得できます。

    ```python
    hoge = wcli.child("hoge") # 自分の hoge という名前のデータ
    fuga = wcli.member("foo").child("fuga") # foo というメンバーの fuga というデータ
    ```

</div>

### データ型

<div class="tabbed">

- <b class="tab-title">C++</b>
    例えば child() の代わりに value() でアクセスすると Value 型が返ります。
    他の型についても同様です。

    ```cpp
    webcface::Value hoge = wcli.value("hoge"); // 自分の hoge という名前のValue型データ
    webcface::Value fuga = wcli.member("foo").value("fuga"); // foo というメンバーの fuga というValue型データ
    ```

    <span class="since-c">1.11</span>
    または、Fieldに対して `.value()` でもValue型に変換できます。
    ```cpp
    webcface::Value hoge = wcli.field("hoge").value();
    // webcface::Value hoge = wcli["hoge"].value(); でも同じ
    webcface::Value fuga = wcli.member("foo").field("fuga");
    // webcface::Value fuga = wcli.member("foo")["fuga"].value(); でも同じ
    ```
    
- <b class="tab-title">JavaScript</b>
    例えば child() の代わりに value() でアクセスすると Value 型が返ります。
    他の型についても同様です。

    ```ts
    const hoge = wcli.value("hoge"); // 自分の hoge という名前のValue型データ
    const fuga = wcli.member("foo").value("fuga"); // foo というメンバーの fuga というValue型データ
    ```

    または、Fieldに対して `.value()` でもValue型に変換できます。
    ```ts
    const hoge = wcli.field("hoge").value();
    const fuga = wcli.member("foo").field("fuga");
    ```

- <b class="tab-title">Python</b>
    例えば child() の代わりに value() でアクセスすると Value 型が返ります。
    他の型についても同様です。

    ```python
    hoge = wcli.value("hoge") # 自分の hoge という名前のValue型データ
    fuga = wcli.member("foo").value("fuga") # foo というメンバーの fuga というValue型データ
    ```

    または、Fieldに対して `.value()` でもValue型に変換できます。
    ```python
    hoge = wcli.field("hoge").value()
    fuga = wcli.member("foo").field("fuga")
    ```

</div>

### グループ化

Fieldの名前を半角ピリオドで区切ると、WebUI上ではフォルダアイコンで表示されグループ化されて表示されます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.child("pos.x");
    wcli.child("pos.y");
    wcli.child("pos.z");
    ```
    のように名前を指定する代わりに、 Field::child() または 各種データ型::child() でもグループ内のデータを指定できます。
    ```cpp
    webcface::Field pos = wcli.child("pos");
    pos.child("x"); // => "pos.x"
    pos.child("y"); // => "pos.y"
    pos.child("z"); // => "pos.z"
    // Value型の pos に対して .child("x") をした場合は、
    // Value型の pos.x データになる
    ```
    または `["名前"]` でも同様です。
    ```cpp
    webcface::Field pos = wcli["pos"];
    pos["x"]; // => "pos.x"
    pos["y"]; // => "pos.y"
    pos["z"]; // => "pos.z"
    ```

    \deprecated
    <span class="since-c">1.11</span>
    <del>child() (または`[]`)の引数が数値(または`"1"`のような文字列でも同じ)の場合、</del>
    <del>グループ化ではなく配列としての値代入が優先されます。</del>
    <del>(これはValue型のみの特別な処理です。)</del>  
    <del>ただし以下のような場合は通常の文字列と同様に処理します。</del>
    ```cpp
    wcli.value("data")[0]["a"] = 1; // value("data.0.a") = 1
    ```
    <span class="since-c">2.6</span> child() や `[]` 内に数値を入れられる仕様はdeprecatedです。


- <b class="tab-title">JavaScript</b>
    ```ts
    wcli.child("pos.x");
    wcli.child("pos.y");
    wcli.child("pos.z");
    ```
    のように名前を指定する代わりに、 Field::child() または 各種データ型::child() でもグループ内のデータを指定できます。
    ```ts
    const pos = wcli.child("pos");
    pos.child("x"); // => "pos.x"
    pos.child("y"); // => "pos.y"
    pos.child("z"); // => "pos.z"
    // Value型の pos に対して .child("x") をした場合は、
    // Value型の pos.x データになる
    ```

- <b class="tab-title">Python</b>
    ```python
    wcli.child("pos.x")
    wcli.child("pos.y")
    wcli.child("pos.z")
    ```
    のように名前を指定する代わりに、 Field::child() または 各種データ型::child() でもグループ内のデータを指定できます。
    ```python
    pos = wcli.child("pos")
    pos.child("x") # => "pos.x"
    pos.child("y") # => "pos.y"
    pos.child("z") # => "pos.z"
    # Value型の pos に対して .child("x") をした場合は、
    # Value型の pos.x データになる
    ```

</div>



\note
(serverが<span class="since-c">1.10</span>以降の場合)
データの名前を半角ピリオドから始めると、Entryが他クライアントに送信されなくなります。
(WebUI上に表示することなくデータを送ることができます)  
半角ピリオド2つから始まる名前はwebcface内部の処理で利用する場合があるので使用しないでください。  
Text、Funcなど他のデータ型についても同様です。


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [4-2. Member](42_member.md) | [5-1. Value](51_value.md) |

</div>
