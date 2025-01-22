# 4-3. Field

\tableofcontents
\since
<span class="since-c">1.11</span>
<span class="since-js">1.10</span>
<span class="since-py">3.1</span>
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
    Client.child() または Member.child() に名前を指定することで Field オブジェクトを取得できます。

    ```ts
    const hoge = wcli.child("hoge"); // 自分の hoge という名前のデータ
    const fuga = wcli.member("foo").child("fuga"); // foo というメンバーの fuga というデータ
    ```

- <b class="tab-title">Python</b>
    Client.child() または Member.child() に名前を指定することで Field オブジェクトを取得できます。

    ```python
    hoge = wcli.child("hoge") # 自分の hoge という名前のデータ
    fuga = wcli.member("foo").child("fuga") # foo というメンバーの fuga というデータ
    ```

</div>

### データ型

Field型のままではデータの送信や受信はできません。
各種データ型のメソッドを使う必要があります。

<div class="tabbed">

- <b class="tab-title">C++</b>
    例えば child() の代わりに value() でアクセスすると Value 型が返ります。
    他の型についても同様です。

    ```cpp
    webcface::Value hoge = wcli.value("hoge"); // 自分の hoge という名前のValue型データ
    webcface::Value fuga = wcli.member("foo").value("fuga"); // foo というメンバーの fuga というValue型データ
    ```

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

![value_child](https://github.com/na-trium-144/webcface/raw/main/docs/images/value_child.png)

\note
ROSのTopicではPointやTransformなど目的に応じてさまざまな型が用意されていますが、
WebCFaceではそういう場合はValueを複数用意して送信することを想定しています。

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
    <del>Value::child() (または`[]`)の引数が数値(または`"1"`のような文字列でも同じ)の場合、</del>
    <del>グループ化ではなく配列としての値代入が優先されます。</del>
    <del>(これはValue型のみの特別な処理です。)</del>  
    <del>ただし以下のような場合は通常の文字列と同様に処理します。</del>
    ```cpp
    wcli.value("data")[0]["a"] = 1; // value("data.0.a") = 1
    ```
    <span class="since-c">2.8</span> child() や `[]` 内に数値を入れられる仕様はdeprecatedです。
    (Value型の配列アクセスを除く)

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

## 基本的な通信の仕様

クライアントが送信したデータは、サーバーを経由して別のクライアントに送られます。
(Valueに限らず、これ以降説明する他のデータ型のfieldについても同様です。)

![pub-sub](https://github.com/na-trium-144/webcface/raw/main/docs/images/pub-sub.png)

サーバー→クライアント間では、初期状態ではデータは送信されず、
クライアントがリクエストを送って初めてサーバーからデータが順次送られてくるようになります。

基本的にクライアント→サーバーの方向にはすべてのデータが送信されるのに対し、サーバー→クライアントの方向には必要なデータのみを送信する設計になっていますが、これは前者はlocalhost(サーバーとクライアントが同じPC)のみで、後者はWi-FiやLANでも通信することを想定したものです。
(通信量は増えますがクライアント→サーバーのデータ送信をWi-FiやLAN経由で行うことも可能です)

### 送信

各種データ型のメソッド (Value.set() や View.sync() など) を使って送信するデータをセットし、
その後 Client::sync() をすることで実際にデータが送信されます。

Client::sync() については [4-1. Client](41_client.md) の送受信の章を参照してください。

<span class="since-c">1.8</span>
<span class="since-js">1.4.1</span>
<span class="since-py">1.1.2</span>
クライアント→サーバー間では、同じデータを繰り返し送信した場合は2回目以降はデータを送信しないことで通信量を削減しています。

### 受信

各種データ型のrequestメソッド (Value.request() や View.request() など) を使ってデータをリクエストできます。
(リクエストの送信は Client::sync() とは非同期に行われます。)

データが一度リクエストされた状態であればそれ以降 Client::sync() 時にデータが受信され、
各種データ型のメソッド (Value.tryGet(), get() など) でデータを取得できます。

また、データを一度もリクエストしていない状態で teyGet(), get() などの関数を呼んでデータを取得しようとした際にも自動的にリクエストが送られます。

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

データが送信された際、 Entry としてそのデータの存在が全クライアントに通知されます。
受信したEntryはリクエスト状態とは関係なく取得可能です。

\note
(serverが<span class="since-c">1.10</span>以降の場合)
データの名前を半角ピリオドから始めると、Entryが他クライアントに送信されなくなります。
(WebUI上に表示することなくデータを送ることができます)  

\warning
半角ピリオド2つから始まる名前はwebcface内部の処理で利用する場合があるので使用しないでください。

<div class="tabbed">

- <b class="tab-title">C++</b>
    例えば <del>Member::values() で</del>
    <span class="since-c">1.6</span>
    Member::valueEntries() でそのMemberが送信しているvalueのリストが得られます。

    [Text](52_text.md), [Func](53_func.md), [View](54_view.md)
    など他のデータ型に関しても同様に `textEntries()`, `funcEntries()`, `viewEntries()` などで取得できます。

    ```cpp
    for(const webcface::Value &v: wcli.member("foo").valueEntries()){
        // ...
    }
    ```

    <span class="since-c">2.1</span>
    Value::exists() でそのデータが送信されているかどうかを確認できます。
    tryGet() と違い、データそのものを受信するリクエストは送られません。
    他のデータ型に関しても同様に `Text::exists()`, `Func::exists()`, `View::exists()` などが使えます。

    <span class="since-c">1.11</span>
    Field::valueEntries() でそのfield以下のvalueのみが得られます
    (Textなど他の型についても同様)
    ```cpp
    std::vector<webcface::Value> values = wcli.member("foo").field("pos").valueEntries();
    // pos.x, pos.y などのvalueが得られる
    ```

    <span class="since-c">2.6</span>
    Field::childrenRecurse() ですべてのデータ型についてそのfield以下に存在するentryを確認できます。
    ```cpp
    std::vector<webcface::Field> values = wcli.member("foo").field("pos").childrenRecurse();
    // pos.x, pos.y などのデータ(型に関係なく)のリストが得られる
    ```
    また、Field::children() では次のピリオドで区切られているグループを取得できます。
    ```cpp
    std::vector<webcface::Field> values = wcli.member("foo").field("pos").children();
    // 例えば、 pos.x, pos.y, pos.vel.x, pos.vel.y が存在した場合、
    // pos.x, pos.y, pos.vel が得られる
    ```
    Field::hasChildren() でそのfield以下にデータが存在するかを確認できます。

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
    <del>Member.values()</del>
    <span class="since-js">1.10</span> Member.valueEntries() でそのMemberが送信しているvalueのリストが得られます  
    ```js
    for(const v of wcli.member("foo").values()){
        // ...
    }
    ```

    [Text](52_text.md), [Func](53_func.md), [View](54_view.md)
    など他のデータ型に関しても同様に `textEntries()`, `funcEntries()`, `viewEntries()` などで取得できます。

    <span class="since-js">1.8</span>
    Value.exists() でそのデータが送信されているかどうかを確認できます。
    tryGet() と違い、データそのものを受信するリクエストは送られません。
    他のデータ型に関しても同様に `Text.exists()`, `Func.exists()`, `View.exists()` などが使えます。

    <span class="since-js">1.10</span>
    Field.valueEntries() でそのfield以下のvalueのみが得られます
    (Textなど他の型についても同様)
    ```ts
    const values = wcli.member("foo").field("pos").valueEntries();
    // pos.x, pos.y などのvalueが得られる
    ```

    <span class="since-js">1.10</span>
    Field.childrenRecurse() ですべてのデータ型についてそのfield以下に存在するentryを確認できます。
    ```ts
    const values = wcli.member("foo").field("pos").childrenRecurse();
    // pos.x, pos.y などのデータ(型に関係なく)のリストが得られる
    ```
    また、Field.children() では次のピリオドで区切られているグループを取得できます。
    ```ts
    const values = wcli.member("foo").field("pos").children();
    // 例えば、 pos.x, pos.y, pos.vel.x, pos.vel.y が存在した場合、
    // pos.x, pos.y, pos.vel が得られる
    ```
    Field.hasChildren() でそのfield以下にデータが存在するかを確認できます。


- <b class="tab-title">Python</b>
    <del>Member.values()</del>
    <span class="since-py">1.1</span>
    Member.value_entries()
    でそのMemberが送信しているvalueのリストが得られます  

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

    <span class="since-py">3.1</span>
    Field.value_entries() でそのfield以下のvalueのみが得られます
    (Textなど他の型についても同様)
    ```python
    values = wcli.member("foo").field("pos").value_entries()
    # pos.x, pos.y などのvalueが得られる
    ```

    <span class="since-py">3.1</span>
    Field.children(recurse=True) ですべてのデータ型についてそのfield以下に存在するentryを確認できます。
    ```python
    values = wcli.member("foo").field("pos").children(recurse=True)
    # pos.x, pos.y などのデータ(型に関係なく)のリストが得られる
    ```
    また、Field.children(recurse=False) では次のピリオドで区切られているグループを取得できます。
    ```python
    values = wcli.member("foo").field("pos").children(recurse=False);
    # 例えば、 pos.x, pos.y, pos.vel.x, pos.vel.y が存在した場合、
    # pos.x, pos.y, pos.vel が得られる
    ```
    Field.has_children() でそのfield以下にデータが存在するかを確認できます。

</div>

### Entry イベント

他のメンバーが新しくデータを追加したときに呼び出されるコールバックを設定できます。
ValueEntry, TextEntry など各データ型にそれぞれあります。

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
        * そのため、すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
        Member名がわかっていれば<del>初回の Client::sync()</del> Client::start() または waitConnection() 前に設定してください。
        * すべてのメンバーのすべてのデータに対してコールバックが呼ばれるようにしたい場合は、 Client::onMemberEntry() イベントのコールバックの中で各種イベントを設定すればよいです。

    <span></span>

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
        * そのため、すべてのデータに対してコールバックが呼ばれるようにしたい場合は、
        Member名がわかっていれば wcfStart() または wcfWaitConnection() 前に設定してください。
        * すべてのメンバーのすべてのデータに対してコールバックが呼ばれるようにしたい場合は、 MemberEntryイベントのコールバックの中で各種イベントを設定すればよいです。

    <span></span>

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

    <span></span>

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

### Change イベント, Sync イベント

受信したデータが変化したときにコールバックを呼び出すことができます。
Change イベントにコールバックを設定するとget()やrequest()を呼ばなくても自動的にその値がリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Sync イベントが使えます。

イベントの詳細な使い方はonMemberEntryと同様です([Member](./42_member.md) のページを参照してください)。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <span class="since-c">2.0</span>
    Value::onChange(), Member::onSync() でコールバックを設定できます。
    他のデータ型についても同様です。

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

    他のデータ型についても同様です。

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
    他のデータ型についても同様です。

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
    他のデータ型についても同様です。

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
| [4-2. Member](42_member.md) | [5-1. Value](51_value.md) |

</div>
