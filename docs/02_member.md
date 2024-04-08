# Member

\tableofcontents
\sa
* C++ webcface::Member (`webcface/member.h`)
* JavaScript [Member](https://na-trium-144.github.io/webcface-js/classes/Member.html)
* Python [webcface.Member](https://na-trium-144.github.io/webcface-python/webcface.member.html#webcface.member.Member)

WebCFaceではサーバーに接続されたそれぞれのクライアントを Member と呼びます。
(たぶんROSでいうと Node に相当します)

データを受信する時など、Memberを指すために使用するのがMemberクラスです。
Client::member() で取得できます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    webcface::Member member_foo = wcli.member("foo");
    ```
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Member } from "webcface";

    const memberFoo: Member = wcli.member("foo");
    ```
- <b class="tab-title">Python</b>
    ```python
    member_foo = wcli.member("foo");
    ```

</div>

これは`foo`という名前のMember(=Clientのコンストラクタに`foo`を入力したクライアント)を指します。

Memberクラスから実際にそれぞれのデータにアクセスする方法は次ページ以降で説明します。

このクライアント自身もMemberの1つですが、Client自体がMemberを継承したクラスになっているので、直接Clientのオブジェクト(wcli)に対して操作すればよいです。

\note
`member()` の引数に自身の名前を入れると、Clientオブジェクトに直接アクセスする場合と同様です。  
<span class="since-c">1.7</span> 引数に空文字列を入れても同様です。

## members

Client::members() で現在接続されているメンバーのリストが得られます
(無名のものと、自分自身を除く)

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    for(const webcface::Member &m: wcli.members()){
        // ...
    }
    ```
- <b class="tab-title">JavaScript</b>
    ```js
    for(const m of wcli.members()){
        // ...
    }
    ```
- <b class="tab-title">Python</b>
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

Client::onMemberEntry() で新しいメンバーが接続されたときのイベントにコールバックを設定できます

このクライアントが接続する前から存在したメンバーについては start() 後に一度に送られるので、
コールバックの設定はstart()より前に行うと良いです。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.onMemberEntry().appendListener([](webcface::Member m){/* ... */});
    ```
    C++では EventTarget クラスのオブジェクトを返します。
    内部ではイベントの管理に [eventpp](https://github.com/wqking/eventpp) ライブラリを使用しており、EventTargetは eventpp::EventDispatcher のラッパーとなっています。

    `appendListener()`, `prependListener()` でコールバックを追加できます。
    また`insertListener()`でこれまでに追加されたコールバックのリストの途中にコールバックを挿入したり、
    `removeListener()` でコールバックを削除したりできます。
    より詳細な使い方はeventppのドキュメントを参照してください。

    \note eventppのEventDispatcherやCallbackListクラスにそのままアクセスできるようにはしていないため、
    eventppのユーティリティ機能はwebcfaceのイベントに対しては使用できなくなっています
    (todo?)

- <b class="tab-title">JavaScript</b>
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
    ```python
    def member_entry(m: webcface.Member):
        pass

    wcli.on_member_entry.connect(member_entry)
    ```
    Pythonでは client.on_member_entry プロパティが [blinker](https://pypi.org/project/blinker/) ライブラリの signal を返します。  
    `connect(関数)` でコールバックを設定できます。

</div>

これ以降の章でもいくつかイベントが登場しますが、いずれもこれと同様の実装、使い方になっています。

## クライアントの情報

Member::libVersion(), Member::libName(), Member::remoteAddr() でクライアントの情報を取得できます。
(WebUI の Connection Info に表示されているのと同じ情報が取得できます)

## ping

Member::pingStatus() でそのクライアントの通信速度を取得できます。(int型で、単位はms)
通信速度の情報は5秒に1回更新され、更新されたときにonPingイベントが発生します


<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.member("foo").onPing().appendListener([](webcface::Member m){
        std::cout << m.name() << ": " << m.pingStatus() << " ms" << std::endl;
    });
    ```
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Member } from "webcface";
    wcli.member("foo").onPing.on((m: Member) => {
        console.log(`${m.name}: ${m.pingStatus} ms`);
    });
    ```
- <b class="tab-title">Python</b>
    ```python
    def ping_update(m: webcface.Member):
        print(f"{m.name}: {m.ping_status} ms")
    wcli.member("foo").on_ping.connect(ping_update)
    ```

</div>

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Client](01_client.md) | [Value](10_value.md) |

</div>
