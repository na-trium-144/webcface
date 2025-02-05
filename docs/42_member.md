# 4-2. Member

\tableofcontents
\sa
* C++ webcface::Member (`webcface/member.h`)
* JavaScript [Member](https://na-trium-144.github.io/webcface-js/classes/Member.html)
* Python [webcface.Member](https://na-trium-144.github.io/webcface-python/webcface.member.html#webcface.member.Member)

WebCFaceではサーバーに接続されたそれぞれのクライアントを Member と呼びます。
(たぶんROSでいうと Node に相当します)

データを受信する時など、Memberを指すために使用するのがMemberクラスです。
Client::member() で取得できます。
(C言語のAPIを除く)

<div class="tabbed">

- <b class="tab-title">C++</b>
    `foo`という名前のMember (=Clientのコンストラクタに`foo`を入力したクライアント)
    にアクセスするには、
    ```cpp
    webcface::Member member_foo = wcli.member("foo");
    ```

    \note
    * `member()` の引数に自身の名前を入れると、Clientオブジェクトに直接アクセスする場合と同様そのクライアント自身を指します。
        * <span class="since-c">1.7</span>
        引数に空文字列を入れても同様です。

- <b class="tab-title">JavaScript</b>
    `foo`という名前のMember (=Clientのコンストラクタに`foo`を入力したクライアント)
    にアクセスするには、
    ```ts
    import { Member } from "webcface";

    const memberFoo: Member = wcli.member("foo");
    ```

    \note
    * `member()` の引数に自身の名前を入れると、Clientオブジェクトに直接アクセスする場合と同様そのクライアント自身を指します。
        * <span class="since-js">1.7</span>
        引数に空文字列を入れても同様です。

- <b class="tab-title">Python</b>
    `foo`という名前のMember (=Clientのコンストラクタに`foo`を入力したクライアント)
    にアクセスするには、
    ```python
    member_foo = wcli.member("foo");
    ```

</div>

Memberクラスから実際にそれぞれのデータにアクセスする方法は次ページ以降で説明します。

このクライアント自身もMemberの1つですが、Client自体がMemberを継承したクラスになっているので、直接Clientのオブジェクト(wcli)に対して操作すればよいです。

## members

現在サーバーに接続されているメンバーのリストが得られます
(無名のものと、自分自身を除く)

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::members() で取得できます。

    ```cpp
    for(const webcface::Member &m: wcli.members()){
        // ...
    }
    ```

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfMemberList`, `wcfMemberListW` にchar\*の配列とサイズを渡すと、メンバーの一覧を取得できます。
    ```c
    const char *member_list[10];
    int actual_member_num;
    wcfMemberList(wcli, member_list, 10, &actual_member_num);
    // member_list[0] から member_list[actual_member_num - 1] (10以上だった場合はmember_list[9]まで) にメンバー名の文字列が入っている
    ```
    それぞれのメンバー名の文字列は、 wcfClose() するまではfreeされません。

- <b class="tab-title">JavaScript</b>
    Client.members() で取得できます。

    ```js
    for(const m of wcli.members()){
        // ...
    }
    ```
    
- <b class="tab-title">Python</b>
    Client.members() で取得できます

    ```python
    for m in wcli.members():
        # ...
    ```

</div>

## Field系クラスの扱いについて

Memberクラスおよびこれ以降説明する各種データ型のクラス (いずれも webcface::Field を継承している) について、

* それぞれコンストラクタが用意されていますが、正しくClientクラスから生成したオブジェクトでないと内部のデータにアクセスしようとするときに std::runtime_error (pythonでは RuntimeError) を投げます。
* 構築元のClientの寿命が切れた後に操作しようとすると同様にstd::runtime_errorを投げます。
* オブジェクトのコピー、ムーブは可能です。

## Event

新しいメンバーが接続されたときに呼び出されるコールバックを設定できます

このクライアントが接続する前から存在したメンバーについては start(), waitConnection() 時に一度に送られるので、
コールバックの設定はstart()より前に行うと良いです。

<div class="tabbed">

- <b class="tab-title">C++</b>
    <span class="since-c">2.0</span>
    引数にMemberを受け取る関数オブジェクトを Client::onMemberEntry() に設定することができます。
    新しく接続したMemberの情報が引数に渡されます。
    ```cpp
    wcli.onMemberEntry([](webcface::Member m){/* ... */});
    ```
    * <span class="since-c">2.0</span>
    Client::waitConnection() を使う場合、
    クライアントはサーバーに接続し、発見したすべてのメンバーについてonMemberEntryコールバックを呼んでからreturnします。

    \note
    <span class="since-c">2.0</span>
    onMemberEntryに限らず、webcfaceが受け取る関数オブジェクトは基本的にコピーではなくムーブされます。

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfMemberEntryEvent`, `wcfMemberEntryEventW` で引数に const char \* と void \* をとる関数ポインタをコールバックとして設定できます。  
    新しく接続したMemberの名前が引数に渡されます。
    void\*引数には登録時に任意のデータのポインタを渡すことができます。(使用しない場合はNULLでよいです。)
    ```c
    void callback_member_entry(const char *name, void *user_data_p) {
        struct UserData *user_data = (struct UserData *)user_data_p;
        // ...
    }
    struct UserData user_data = {...};
    wcfMemberEntryEvent(wcli, callback_member_entry, &user_data);
    ```

    wcfWaitConnection() を使う場合、
    クライアントはサーバーに接続し、発見したすべてのメンバーについてMemberEntryEventのコールバックを呼んでからreturnします。

- <b class="tab-title">JavaScript</b>
    引数にMemberを受け取る関数を Client.onMemberEntry() に設定することができます。
    新しく接続したMemberの情報が引数に渡されます。
    ```ts
    import { Member } from "webcface";

    wcli.onMemberEntry.on((m: Member) => { /* ... */ });
    ```

    イベントの管理には [eventemitter3](https://www.npmjs.com/package/eventemitter3) ライブラリを使用しており、
    Client.onMemberEntry プロパティが返す [EventTarget](https://na-trium-144.github.io/webcface-js/classes/EventTarget.html) クラスのオブジェクトがEventEmitterのラッパーになっています。

    `on(関数)` または `addListener(関数)` でコールバックを設定し、
    `off(関数)` または `removeListener(関数)` で解除したりできます。
    また `once(関数)` で1回だけ実行されるコールバックを設定できます。
    
- <b class="tab-title">Python</b>
    引数にMemberを受け取る関数を Client.on_member_entry() に設定することができます。
    新しく接続したMemberの情報が引数に渡されます。

    <span class="since-py">2.0</span>
    ```python
    def member_entry(m: webcface.Member):
        pass

    wcli.on_member_entry(member_entry)
    ```

    デコレータとして使うこともできます。
    ```python
    @wcli.on_member_entry
    def member_entry(m: webcface.Member):
        pass
    ```

    <span class="since-py">2.0</span>
    Client.wait_connection() を使う場合、
    クライアントはサーバーに接続し、発見したすべてのメンバーについてon_member_entryコールバックを呼んでからreturnします。


</div>

<details><summary>C++ 〜ver1.11の仕様</summary>

```cpp
wcli.onMemberEntry().appendListener([](webcface::Member m){/* ... */});
```
C++では EventTarget クラスのオブジェクトを返します。
内部ではイベントの管理に [eventpp](https://github.com/wqking/eventpp) ライブラリを使用しており、EventTargetは eventpp::CallbackList のラッパーとなっています。

`appendListener()`, `prependListener()` でコールバックを追加できます。
また`insertListener()`でこれまでに追加されたコールバックのリストの途中にコールバックを挿入したり、
`removeListener()` でコールバックを削除したりできます。

<span class="since-c">1.7</span>
appendListener, prependListener ではコールバックの引数が不要な場合は引数のない関数も渡すことができます。

<span class="since-c">1.11</span>
`wcli.onMemberEntry().callbackList()` で[eventpp::CallbackList](https://github.com/wqking/eventpp/blob/master/doc/callbacklist.md)のインスタンスが得られ、
CounterRemover, ConditionalRemover などeventppに用意されているさまざまなユーティリティ機能を使用できます。
より詳細な使い方はeventppのドキュメントを参照してください。
(この場合コールバックの引数は省略できません)
```cpp
wcli.onMemberEntry().callbackList().append([](webcface::Member m){/* ... */});

// CounterRemoverを使って1回呼び出されたらコールバックを削除する:
eventpp::counterRemover(wcli.onMemberEntry().callbackList())
    .append([](webcface::Member m){/* ... */}, 1);
```

\note
ver1.10以前は eventpp::EventDispatcher を使用していたため、
EventTargetでの関数名(appendListenerなど)はCallbackListではなくEventDispatcherのものに従っている

これ以降の章でもいくつかイベントが登場しますが、いずれもこれと同様の実装、使い方になっています。

</details>

<details><summary>Python 〜ver1.1の仕様</summary>

```python
def member_entry(m: webcface.Member):
    pass

wcli.on_member_entry.connect(member_entry)
```
Pythonでは client.on_member_entry プロパティが [blinker](https://pypi.org/project/blinker/) ライブラリの signal を返します。  
`connect(関数)` でコールバックを設定できます。

</details>

## クライアントの情報

以下のようなクライアントの情報を取得できます。
(WebUI の Connection Info に表示されているのと同じ情報が取得できます)

* libVersion でWebCFaceライブラリのバージョンを取得できます。
* libName で使用しているWebCFaceライブラリを判別できます。
C++のライブラリは `"cpp"`, Pythonのライブラリ(webcface-python)は`"python"`, JavaScriptのライブラリ(webcface-js)は`"js"`を返します。
* remoteAddr はサーバーから見た各メンバーのIPアドレスです。
    * <span class="since-c">1.11</span> Unixドメインソケットで接続している場合空文字列が返ります。

<div class="tabbed">

- <b class="tab-title">C++</b>
    `member.libVersion()`, `member.libName()`, `member.remoteAddr()` で取得できます。

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfMemberLibVersion(wcli, name)`, `wcfMemberLibName(wcli, name)`, `wcfMemberRemoteAddr(wcli, name)` で取得できます。

- <b class="tab-title">JavaScript</b>
    `member.libVersion`, `member.libName`, `member.remoteAddr` で取得できます。

- <b class="tab-title">Python</b>
    `member.lib_version`, `member.lib_name`, `member.remote_addr` で取得できます。
    
</div>

## ping

クライアントの通信速度を取得できます。(単位はms)
ここでは通信速度とはサーバーとクライアントの間で1往復データを送受信するのにかかる遅延です。

通信速度の情報は5秒に1回更新され、更新されたときにonPingイベントが発生します

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::pingStatus() でmemberの通信速度(int型)を取得できます。
    デフォルトの状態ではpingの情報は受信しておらずstd::nulloptを返しますが、
    pingStatusに1回アクセスすることでpingの情報がリクエストされ、それ以降は値が送られてくるようになります。

    また、 Member::onPing() でpingの情報が更新された時に実行するコールバックを設定できます。
    onPing()を使うことでもリクエストが送られます。

    <span class="since-c">2.0</span>
    ```cpp
    wcli.member("foo").onPing([](webcface::Member m){
        std::cout << m.name() << ": " << m.pingStatus() << " ms" << std::endl;
    });
    ```
    * ver1.11以前では `onPing().appendListener(...)`
    * <span class="since-c">1.7</span>
    <del>appendListener, prependListener では</del> コールバックの引数が不要な場合は引数のない関数も渡すことができます。
    * <span class="since-c">1.11</span>
    <del>onMemberEntry() と同様、 callbackList() でCallbackListにアクセスできます。</del>
    * <span class="since-c">2.0</span>
    自分自身のping値も取得できるようになりました。(`wcli.pingStatus()`, `wcli.onPing(...)`)

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfMemberPingStatus`, `wcfMemberPingStatusW` でmemberの通信速度を取得できます。
    ```c
    int ping_status;
    wcfMemberPingStatus(wcli, name, &ping_status);
    ```

    また、 `wcfMemberPingEvent`, `wcfMemberPingEventW` で引数に const char \* と void \* をとる関数ポインタをコールバックとして設定できます。
    void\*引数には登録時に任意のデータのポインタを渡すことができます。(使用しない場合はNULLでよいです。)
    ```c
    void callback_ping(const char *name, void *user_data_p) {
        struct UserData *user_data = (struct UserData *)user_data_p;
        // ...
    }
    struct UserData user_data = {...};
    wcfMemberPingEvent(wcli, callback_ping, &user_data);
    ```

- <b class="tab-title">JavaScript</b>
    Member.pingStatus でmemberの通信速度を取得できます。
    デフォルトの状態ではpingの情報は受信しておらずnullを返しますが、
    pingStatusに1回アクセスすることでpingの情報がリクエストされ、それ以降は値が送られてくるようになります。

    また、 Member.onPing でpingの情報が更新された時に実行するコールバックを設定できます。
    onPingを使うことでもリクエストが送られます。
    ```ts
    import { Member } from "webcface";
    wcli.member("foo").onPing.on((m: Member) => {
        console.log(`${m.name}: ${m.pingStatus} ms`);
    });
    ```
    * <span class="since-js">1.7</span>
    自分自身のping値も取得できるようになりました。(`wcli.pingStatus`, `wcli.onPing`)
    * <span class="since-js">1.8</span>
    Member.requestPingStatus() で明示的にリクエストを送ることもできます。

- <b class="tab-title">Python</b>
    Member.ping_status でmemberの通信速度(int型)を取得できます。
    デフォルトの状態ではpingの情報は受信しておらずNoneを返しますが、
    ping_statusに1回アクセスすることでpingの情報がリクエストされ、それ以降は値が送られてくるようになります。

    また、 Member.on_ping() でpingの情報が更新された時に実行するコールバックを設定できます。
    on_ping()を使うことでもリクエストが送られます。

    <span class=since-py>2.0</span>
    ```python
    def ping_update(m: webcface.Member):
        print(f"{m.name}: {m.ping_status} ms")
    wcli.member("foo").on_ping(ping_update)
    ```
    * ver1.1以前は `on_ping.connect(...)`
    * <span class=since-py>2.0</span>
    on_member_entryと同様、デコレータとして使うこともできます。
    * <span class="since-py">2.0</span>
    自分自身のping値も取得できるようになりました。(`wcli.ping_status`, `wcli.on_ping(...)`)
    * <span class="since-py">2.0</span>
    Member.request_ping_status() で明示的にリクエストを送ることもできます。


</div>

\warning
<span class="since-c">2.0</span>
各クライアントがPingに応答する処理はClient::sync()の中で行われるため、
sync() を呼ぶ頻度が遅いとPingの応答も遅くなり通信速度の表示に影響します。
(例えば100msに1回 sync() を呼ぶ場合通信遅延が100msあるように見える可能性があります)
[4-1. Client](41_client.md)の送受信の章にあるようにloopSync()などを使っている場合は問題ありません。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [4-1. Client](41_client.md) | [4-3. Field](43_field.md) |

</div>
