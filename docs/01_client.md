# Client

\tableofcontents
\sa
* C++ webcface::Client (`webcface/client.h`)
* C Reference: c_wcf/client.h
* JavaScript [Client](https://na-trium-144.github.io/webcface-js/classes/Client.html)
* Python [webcface.Client](https://na-trium-144.github.io/webcface-python/webcface.client.html#webcface.client.Client)

WebCFaceのメインとなるクラスです。

## 接続

~~Client オブジェクトを作るとサーバーへの接続を開始します。~~  
<span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
Client オブジェクトを作り、start() を呼ぶことでサーバーへの接続を開始します。

バックグラウンド(別スレッド)で接続が完了するまでの間はデータの受信などはできません。

デフォルトでは`127.0.0.1`(そのpc自身)の7530ポートに接続を試みますが、サーバーのポートを変更していたり別のpcに接続する場合はサーバーのアドレスとポートを指定できます。

コンストラクタに指定するのはこのクライアントの名前(Memberの名前)です。
同時に接続している他のクライアントと名前が被らないようにしてください。同じ名前で複数のクライアントが接続した場合正常に通信できない場合があります。
(おそらく後に接続したクライアントが優先される?)

\note
コンストラクタに名前を指定しない、または空文字列の場合、読み取り専用モードになります。  
この場合データを送信することができなくなりますが、名前が被っても正常に通信できるため、同じプログラムを複数起動することができます
(WebUIではこの仕様を使っています)

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    #include <webcface/client.h>
    // または #include <webcface/webcface.h> (すべての機能をinclude)

    webcface::Client wcli("sample");
    // アドレスを指定する場合
    // webcface::Client wcli("sample", "192.168.1.1", 7530);

    // wstringの場合 (ver2.0〜, 詳細はこのページのEncodingの章を参照)
    // webcface::Client wcli(L"sample");
    // webcface::Client wcli(L"sample", L"192.168.1.1", 7530);

    wcli.start();
    // または wcli.waitConnection();
    ```

    * <del>`start()` の代わりに `waitConnection()` を使うと接続が完了するまで待機することができます。</del>
    * <span class="since-c">2.0</span>
    `start()` の代わりに `waitConnection(interval)` を使うと接続が完了してEntry(=他のクライアントが送信しているデータのリスト)をすべて受信するまで待機することができます。
        * waitConnectionは通信完了までの間interval間隔でrecv()を呼び出します。(デフォルト: 100μs) 詳細は後述の送受信の説明を参照
    * 接続できているかどうかは `wcli.connected()` で取得できます。
    * 通信が切断された場合は自動で再接続します。
        * <span class="since-c">1.11.1</span>
    `autoReconnect(false)`をすると自動で再接続しなくなります。
    その場合`waitConnection()`は1度だけ接続を試行し失敗してもreturnするという挙動になります。

    \note
    * <span class="since-c">1.11</span>
    接続先のアドレスが`127.0.0.1`の場合はTCPポートと別にUnixドメインソケットへの接続も試行し、
    接続できた場合はUnixドメインソケットの接続を優先して使用します。
        * WSL1上のクライアントは `/mnt/c/ProgramData/webcface/ポート番号.sock` への接続も試行します。これによりWSL1とWindowsの間ではどちらでサーバーを建ててもどちらのクライアントからも相互に接続できます。
    * <span class="since-c">1.11</span>
    WSL2ではUnixドメインソケットを使った相互接続はできませんが、WSL2のクライアントは接続先のアドレスが`127.0.0.1`の場合に限りホストのWindowsのIPアドレスのTCPポートへも接続を試行します。これによりWSL2とWindowsの間でも相互に通信が可能です。
    * ~~Clientはstart()時に2つのスレッド(std::thread)を建てます。~~
    ~~1つはWebSocketの送受信処理用で、送信用キューにあるメッセージを送信し、受信したら受信用キューに入れます。~~
    ~~もう1つは受信したメッセージをパースしてコールバックを呼んだりといった処理をします。~~
        * ~~Callメッセージ(Func呼び出しのメッセージ)を受信したときと、自分自身のFuncに対してrunAsync()を呼び出したときは、その呼び出し1回ごとに新しいスレッドを建てその中で関数を実行します。~~
    * <span class="since-c">2.0</span> Clientはstart()時に2〜3つのスレッド(std::thread)を建てます。
        * 1つは接続処理用で、通信が切断されたときに再接続を行います。接続できている間は何もしません。
        * 1つは送信用で、送信用キューにデータが追加されたらそれを送信します。
        * もう1つは受信用で、autoRecv()を使用する場合このスレッドがデータを受信してコールバックを呼びます(詳細はこのページの送受信の章を参照)


- <b class="tab-title">C</b>
    ```c
    #include <webcface/wcf.h>

    wcfClient *wcli = wcfInitDefault("sample");
    wcfStart(wcli);
    ```
    でclientを生成し、接続します。  
    <span class="since-c">1.7</span> 引数にNULLを渡した場合空文字列と同様になります

    サーバーのアドレスとポートを指定したい場合`wcfInit()`を使います
    ```c
    #include <webcface/wcf.h>

    wcfClient *wcli = wcfInit("sample", "192.168.1.1", 7530);
    wcfStart(wcli);
    ```

    * <span class="since-c">2.0</span>
    ワイド文字列を使用したい場合はそれぞれ `wcfInitDefaultW()`, `wcfInitW()`
    (詳細はこのページのEncodingの章を参照)
    * <span class="since-c">2.0</span>
    `wcfStart()` の代わりに `wcfWaitConnection(interval)` を使うと接続が完了してEntry(=他のクライアントが送信しているデータのリスト)をすべて受信するまで待機することができます。
        * wcfWaitConnectionは通信完了までの間interval(μs)間隔でrecv()を呼び出します。
        (C++側のデフォルトは100なので、理由がなければ100程度を指定しておけばよいです。
        詳細は後述の送受信の説明を参照)
    * 接続できているかどうかは `wcfIsConnected(wcli)` で取得できます。
    * 通信が切断された場合は自動で再接続します。

    \note
    * <span class="since-c">1.11</span>
    接続先のアドレスが`127.0.0.1`の場合(wcfInitDefaultを使用した場合も含む)はTCPポートと別にUnixドメインソケットへの接続も試行し、
    接続できた場合はUnixドメインソケットの接続を優先して使用します。
        * WSL1上のクライアントは /mnt/c/ProgramData/webcface/ポート番号.sock への接続も試行します。これによりWSL1とWindowsの間ではどちらでサーバーを建ててもどちらのクライアントからも相互に接続できます。
    * <span class="since-c">1.11</span>
    WSL2ではUnixドメインソケットを使った相互接続はできませんが、WSL2のクライアントは接続先のアドレスが`127.0.0.1`の場合に限りホストのWindowsのIPアドレスのTCPポートへも接続を試行します。これによりWSL2とWindowsの間でも相互に通信が可能です。
    * ~~ClientはwcfStart()時に2つのスレッド(std::thread)を建てます。~~
    ~~1つはWebSocketの送受信処理用で、送信用キューにあるメッセージを送信し、受信したら受信用キューに入れます。~~
    ~~もう1つは受信したメッセージをパースしてコールバックを呼んだりといった処理をします。~~
        * ~~Callメッセージ(Func呼び出しのメッセージ)を受信したときと、自分自身のFuncに対してwcfFuncRunAsync()を呼び出したときは、その呼び出し1回ごとに新しいスレッドを建てその中で関数を実行します。~~
    * <span class="since-c">2.0</span> Clientはstart()時に2〜3つのスレッド(std::thread)を建てます。
        * 1つは接続処理用で、通信が切断されたときに再接続を行います。接続できている間は何もしません。
        * 1つは送信用で、送信用キューにデータが追加されたらそれを送信します。
        * もう1つは受信用で、autoRecv()を使用する場合このスレッドがデータを受信してコールバックを呼びます(詳細はこのページの送受信の章を参照)

- <b class="tab-title">JavaScript</b>
    ```ts
    import { Client } from "webcface";

    const wcli = Client("sample");
    // アドレスを指定する場合
    // const wcli = Client("sample", "192.168.1.1", 7530);

    wcli.start();
    ```

    \note
    * 接続できているかどうかは `wcli.connected` で取得できます。
    * 通信が切断された場合は自動で再接続します。

    \warning
    * webブラウザ上でJavaScriptから接続しようとする場合、サーバー側からみたlocalhostではなくブラウザ側からみたlocalhostへ接続しようとするので注意してください ([location.host](https://developer.mozilla.org/ja/docs/Web/API/Location/host) などを接続先アドレスに指定する必要があります)
    
- <b class="tab-title">Python</b>
    ```python
    import webcface

    wcli = webcface.Client("sample")
    # アドレスを指定する場合
    # wcli = webcface.Client("sample", "192.168.1.1", 7530)

    wcli.start()
    # または wcli.wait_connection()
    ```

    \note
    * 接続できているかどうかは `wcli.connected` で取得できます。
    * `start()` の代わりに `wait_connection()` を使うと接続が完了するまで待機することができます。
    * 通信が切断された場合は自動で再接続します。

    <span></span>

</div>

クライアントが正常に接続できると、サーバーのログに
```
[info] Successfully connected and initialized.
```
などと表示されます。
(クライアントの名前を指定しなかった場合は表示されません)

プログラムを起動できたら、WebUIを開いてみましょう。
そして右上のメニューを開き、Clientの初期化時に指定した名前がそこに表示されていれば正しく通信できています。

## Encoding

<span class="since-c">2.0</span>

webcfaceのAPIではほぼすべての関数でマルチバイト文字列(`std::string`, `char *`)の代わりにワイド文字列(`std::wstring`, `wchar_t *`)が使用可能です。

マルチバイト文字列はデフォルトではWindows,Linux,MacOSともにUTF-8エンコーディングとして扱われます。
ただしWindowsでのみClientの初期化前に最初に `webcface::usingUTF8(false)` (Cでは `wcfUsingUTF8(0)`) をすることでANSIエンコーディング(日本語環境ではShiftJIS)として扱われるようにすることもできます。
(その場合webcface内部でUTF-8との変換が行われます)
デフォルトでは `webcface::usingUTF8(true)` (Cでは `wcfUsingUTF8(1)`) が設定されています。

ワイド文字列はWindowsではUTF-16、Linux/MacOSではUTF-32でエンコードされていることを想定しています。
usingUTF8 の設定はワイド文字列の扱いには影響しませんが、
windowsで Client::loggerWStreamBuf(), loggerWOStream() を使用する場合は出力するコンソールのコードページに合わせてください。
(内部でstringに変換してコンソールへ出力されるため)

C++で文字列を返すAPI、およびCのAPI全般ではワイド文字列を使用する関数やstruct名には末尾に`W`が付きます。

## 送受信

<div class="tabbed">

- <b class="tab-title">C++</b>
    * sync() をすることでこれ以降の章で扱う各種データを送信します。
        * sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
        * 送信したいデータがある場合は、メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
        * ~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~
        * <span class="since-c">1.2</span>
        Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。
    * <span class="since-c">2.0</span>
    recv(), waitRecvFor(timeout), waitRecvUntil(timeout) または waitRecv() をすることでデータを受信します。
        * メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
            * sync()と違ってこちらはほぼどんな使い方の場合でも必要になります。
            各種データを受信するだけでなくFuncの呼び出しや逆にFuncを呼び出される場合などにも使われます。
        * データを何も受信しなかった場合、サーバーに接続していない場合、または接続試行中やデータ送信中など受信ができない場合は、timeout経過後にreturnします。
            * recv() は即座にreturnします。
            * waitRecv() は何かデータを受信するまで無制限に待機します。

    ```cpp
    while(true){
        // ...

        wcli.sync();
        wcli.waitRecvFor(std::chrono::milliseconds(100));
    }
    ```

    * <span class="since-c">2.0</span>
    recv() を毎周期呼ぶ代わりに、 autoRecv(true) を最初のstart()前に呼ぶと、別スレッドで一定間隔(100μs)おきに自動でrecvしてくれるようになります。
    (ver1.11以前ではデータの受信は常に別スレッドで行われていたので、その挙動と同じになります)

    \note
    timeoutのない recv() を使う場合は、sync()の前にrecv()を呼んでください。
    (sync()でキューに加えられた送信データの処理中はrecv()の処理ができないため)
    ```cpp
    wcli.recv();
    wcli.sync();
    ```
    一方timeoutのあるwaitRecvを使う場合はsync()を先に呼ばないとデータ送信が遅延します。

    \warning
    * これ以降の章で扱う各種データの受信時にコールバックを設定できますが、それはrecv()を呼んだスレッドで実行されます。
    そのため長時間かかるコールバックを登録した場合、その間recv()を呼んだメインスレッドがブロックされるだけでなく、他のデータの受信もできなくなるため注意してください。

    \warning
    * 1msに1回程度の頻度までは動作確認していますが、
    それ以上短い周期で sync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

- <b class="tab-title">C</b>
    * wcfSync() をすることでこれ以降の章で扱う各種データを送信します。
        * wcfSync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
        メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
        * Funcの呼び出しとデータ受信リクエストの送信は wcfSync() とは非同期に行われるので wcfSync() は不要です。
    * <span class="since-c">2.0</span>
    wcfRecv(), wcfWaitRecvFor(timeout) または wcfWaitRecv() をすることでデータを受信します。
        * データを何も受信しなかった場合、サーバーに接続していない場合、または接続試行中やデータ送信中など受信ができない場合は、timeout経過後にreturnします。
            * wcfRecv() は即座にreturnします。
            * wcfWaitRecv() は何かデータを受信するまで無制限に待機します。
    ```c
    while(1){
        // ...
        wcfSync(wcli);
        wcfRecv(wcli, 100e3); // 100000μs = 100ms
    }
    ```

    * <span class="since-c">2.0</span>
    wcfRecv() を毎周期呼ぶ代わりに、 wcfAutoRecv() を最初のwcfStart()前に呼ぶと、別スレッドで一定間隔おきに自動でrecvしてくれるようになります。
    (ver1.11以前ではデータの受信は常に別スレッドで行われていたので、その挙動と同じになります)

    \note
    timeoutのない wcfRecv() を使う場合は、wcfSync()の前にwcfRecv()を呼んでください。
    (wcfSync()でキューに加えられた送信データの処理中はwcfRecv()の処理ができないため)
    ```cpp
    wcfRecv(wcli, 0);
    wcfSync(wcli);
    ```
    一方timeoutのあるwcfWaitRecvを使う場合はwcfSync()を先に呼ばないとデータ送信が遅延します。

    \warning
    * これ以降の章で扱う各種データの受信時にコールバックを設定できますが、それはwcfRecv()を呼んだスレッドで実行されます。
    そのため長時間かかるコールバックを登録した場合、その間wcfRecv()を呼んだメインスレッドがブロックされるだけでなく、他のデータの受信もできなくなるため注意してください。

    \warning
    * 1msに1回程度の頻度までは動作確認していますが、
    それ以上短い周期で wcfSync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

- <b class="tab-title">JavaScript</b>
    * sync() をすることでこれ以降の章で扱う各種データを送信します。
        * メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
        * ~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~
        * <span class="since-js">1.1</span>
        Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。
    * データの受信処理は非同期で行われます。
    ```ts
    setInterval(() => {
        wcli.sync();
    }, 100);
    ```


- <b class="tab-title">Python</b>
    * sync() をすることでこれ以降の章で扱う各種データを送信します。
        * sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
        メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
        * Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。
    ```python
    while True:
        # ...

        wcli.sync()
        time.sleep(0.1)
    ```

</div>

## 切断する

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.close();
    ```
    \note
    C++ではClientのデストラクタでも自動的に切断します。

- <b class="tab-title">C</b>
    ```c
    wcfClose(wcli);
    ```

- <b class="tab-title">JavaScript</b>
    ```ts
    wcli.close();
    ```

- <b class="tab-title">Python</b>
    ```py
    wcli.close()
    ```

</div>

切断するとサーバーのログに
```
[info] connection closed
```
などと表示されます。

\warning
closeしたあと再度start()を呼んで再接続することはできません。

## バージョン情報

いま使用しているWebCFaceライブラリのバージョンを確認できます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ライブラリのヘッダーファイルのバージョンは  
    `WEBCFACE_VERSION` で文字列として(例: `"1.2.0-ubuntu22.04"`)、  
    `WEBCFACE_VERSION_MAJOR`, `WEBCFACE_VERSION_MINOR`, `WEBCFACE_VERSION_REVISION` で数値として(例: それぞれ `1`, `2`, `0`)  
    取得できます。
    コンパイル時に(`#if` や `if constexpr` で)バージョンによって処理を変えることができます。

    <span class="since-c">1.2</span>
    実行時にリンクしているライブラリのバージョンは  
    webcface::version_s で文字列として(例: `"1.2.0-ubuntu22.04"`)、  
    webcface::version で数値として(例: `{1, 2, 0}`)  
    取得できます。

- <b class="tab-title">C</b>
    ライブラリのヘッダーファイルのバージョンは  
    `WEBCFACE_VERSION` で文字列として(例: `"1.2.0-ubuntu22.04"` )、  
    `WEBCFACE_VERSION_MAJOR`, `WEBCFACE_VERSION_MINOR`, `WEBCFACE_VERSION_REVISION` で数値として(例: それぞれ `1`, `2`, `0` )  
    取得できます。
    コンパイル時に( `#if` で)バージョンによって処理を変えることができます。

- <b class="tab-title">Python</b>
    \since <span class="since-py">1.0.2</span>

    `webcface.__version__` で文字列として(例: `"1.0.2"`)取得できます。
    
</div>


## サーバーの情報

### serverVersion, serverName

* serverVersion でサーバーのバージョンを取得できます。
* serverName は現在は`"webcface"`という文字列しか返しません。
(今後webcface-serverを別言語で実装したりこれと同等の機能をもった別のプログラムを作ることがあったら判別できるようにするためのものです <del>そんなことあるのか?</del>)

<div class="tabbed">

- <b class="tab-title">C++</b>
    `wcli.serverVersion()`, `wcli.serverName()` で取得できます。
- <b class="tab-title">JavaScript</b>
    `wcli.serverVersion`, `wcli.serverName` で取得できます。
- <b class="tab-title">Python</b>
    `wcli.server_version`, `wcli.server_name` で取得できます。
    
</div>

### serverHostName

サーバーを起動しているPCのホスト名を取得できます。
WebUI ver1.7 以降ではWebUIのページタイトルにも表示されています。

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">2.0</span>
    `wcli.serverHostName()` で取得できます。
- <b class="tab-title">JavaScript</b>
    \since <span class="since-js">1.7</span>
    `wcli.serverHostName` で取得できます。

</div>

\warning
Clientの接続が完了し受信するまでは取得できません(空文字列になります)。
受信するのは waitConnection() の待機完了よりも後です。

## ログ出力

\since
<span class="since-c">1.1.7</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>

`WEBCFACE_VERBOSE` 環境変数が存在する場合、WebCFaceの通信に関するログ(接続、切断、メッセージのエラー)が出力されます。
また `WEBCFACE_TRACE` 環境変数が存在すると内部で使用しているlibcurlの出力も表示します。

<span class="since-js">1.1</span>
ではClientのコンストラクタでlogLevelに `"trace"` または `"verbose"` を指定することでも表示できます。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Overview](00_overview.md) | [Member](02_member.md) |

</div>
