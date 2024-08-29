# 4-1. Client

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
    C++のソースコードでは`<webcface/webcface.h>`をincludeするとwebcfaceのすべての機能が使用できます。

    <span class="since-c">1.10</span>
    `<webcface/client.h>`, `<webcface/value.h>`など必要なヘッダファイルだけincludeして使うこともでき、コンパイル時間を短縮できます。

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

    * `Client::start()` の代わりに `Client::waitConnection()` を使うと <del>接続が完了するまで待機することができます。</del>
    * <span class="since-c">2.0</span>
    `Client::start()` の代わりに `Client::waitConnection()` を使うと接続が完了してEntry(=他のクライアントが送信しているデータのリスト)をすべて受信するまで待機することができます。
        * waitConnectionは通信完了までの間loopSync()を呼び出します。 詳細は後述の送受信の説明を参照
    * 接続できているかどうかは `Client::connected()` で取得できます。
    * 通信が切断された場合は自動で再接続します。
        * <span class="since-c">1.11.1</span>
    `Client::autoReconnect(false)`をすると自動で再接続しなくなります。
    その場合`Client::waitConnection()`は1度だけ接続を試行し失敗してもreturnするという挙動になります。

    \note
    * <span class="since-c">1.11</span>
    接続先のアドレスが`127.0.0.1`の場合はTCPポートと別にUnixドメインソケットへの接続も試行し、
    接続できた場合はUnixドメインソケットの接続を優先して使用します。
        * WSL1上のクライアントは `/mnt/c/ProgramData/webcface/ポート番号.sock` への接続も試行します。これによりWSL1とWindowsの間ではどちらでサーバーを建ててもどちらのクライアントからも相互に接続できます。
    * <span class="since-c">1.11</span>
    WSL2ではUnixドメインソケットを使った相互接続はできませんが、WSL2のクライアントは接続先のアドレスが`127.0.0.1`の場合に限りホストのWindowsのIPアドレスのTCPポートへも接続を試行します。これによりWSL2とWindowsの間でも相互に通信が可能です。
    * <del>Clientはstart()時に2つのスレッド(std::thread)を建てます。1つはWebSocketの送受信処理用で、送信用キューにあるメッセージを送信し、受信したら受信用キューに入れます。もう1つは受信したメッセージをパースしてコールバックを呼んだりといった処理をします。</del>
        * <del>Callメッセージ(Func呼び出しのメッセージ)を受信したときと、自分自身のFuncに対してrunAsync()を呼び出したときは、その呼び出し1回ごとに新しいスレッドを建てその中で関数を実行します。</del>
    * <span class="since-c">2.0</span> Clientはstart()時に1つのスレッド(std::thread)を建てます。
        * 送信用キューにデータが追加されたらそれを送信し、データを受信したら受信用キューに入れます。
        また通信が切断されたときに再接続を行います。
        * 受信したデータを処理してコールバックを呼ぶのは別スレッドではありません。
        (詳細はこのページの送受信の章を参照)

    <span></span>

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
    `wcfStart()` の代わりに `wcfWaitConnection()` を使うと接続が完了してEntry(=他のクライアントが送信しているデータのリスト)をすべて受信するまで待機することができます。
        * wcfWaitConnectionは通信完了までの間wcfLoopSync()を呼び出します。
    * 接続できているかどうかは `wcfIsConnected()` で取得できます。
    * 通信が切断された場合は自動で再接続します。

    \note
    * <span class="since-c">1.11</span>
    接続先のアドレスが`127.0.0.1`の場合(wcfInitDefaultを使用した場合も含む)はTCPポートと別にUnixドメインソケットへの接続も試行し、
    接続できた場合はUnixドメインソケットの接続を優先して使用します。
        * WSL1上のクライアントは /mnt/c/ProgramData/webcface/ポート番号.sock への接続も試行します。これによりWSL1とWindowsの間ではどちらでサーバーを建ててもどちらのクライアントからも相互に接続できます。
    * <span class="since-c">1.11</span>
    WSL2ではUnixドメインソケットを使った相互接続はできませんが、WSL2のクライアントは接続先のアドレスが`127.0.0.1`の場合に限りホストのWindowsのIPアドレスのTCPポートへも接続を試行します。これによりWSL2とWindowsの間でも相互に通信が可能です。
    * <del>ClientはwcfStart()時に2つのスレッド(std::thread)を建てます。1つはWebSocketの送受信処理用で、送信用キューにあるメッセージを送信し、受信したら受信用キューに入れます。もう1つは受信したメッセージをパースしてコールバックを呼んだりといった処理をします。</del>
        * <del>Callメッセージ(Func呼び出しのメッセージ)を受信したときと、自分自身のFuncに対してwcfFuncRunAsync()を呼び出したときは、その呼び出し1回ごとに新しいスレッドを建てその中で関数を実行します。</del>
    * <span class="since-c">2.0</span> Clientはstart()時に1つのスレッド(std::thread)を建てます。
        * 送信用キューにデータが追加されたらそれを送信し、データを受信したら受信用キューに入れます。
        また通信が切断されたときに再接続を行います。
        * 受信したデータを処理してコールバックを呼ぶのは別スレッドではありません。
        (詳細はこのページの送受信の章を参照)

    <span></span>

- <b class="tab-title">JavaScript</b>
    * ESMの例
    ```ts
    import { Client } from "webcface";

    const wcli = Client("sample");
    // アドレスを指定する場合
    // const wcli = Client("sample", "192.168.1.1", 7530);

    wcli.start();
    ```
    * <span class="since-js">1.7</span> htmlからCDNを利用する例
    ```html
    <scripts>
        const wcli = webcface.Client("sample");
        // アドレスを指定する場合
        // const wcli = Client("sample", "192.168.1.1", 7530);

        wcli.start();
    </scripts>
    ```

    \note
    以降このドキュメントではESMのコード例のみを載せています。
    CDNのWebCFaceを使う場合は上の例と同様に `webcface.クラス名` と読み替えてください。

    <span></span>

    * 接続できているかどうかは `wcli.connected` で取得できます。
    * 通信が切断された場合は自動で再接続します。

    \warning
    * webブラウザ上でJavaScriptから接続しようとする場合、サーバー側からみたlocalhostではなくブラウザ側からみたlocalhostへ接続しようとするので注意してください ([location.host](https://developer.mozilla.org/ja/docs/Web/API/Location/host) などを接続先アドレスに指定する必要があります)
    
    <span></span>

- <b class="tab-title">Python</b>
    ```python
    import webcface

    wcli = webcface.Client("sample")
    # アドレスを指定する場合
    # wcli = webcface.Client("sample", "192.168.1.1", 7530)

    wcli.start()
    # または wcli.wait_connection()
    ```

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
    ```cpp
    wcli.sync();
    ```

    * Client::sync() をすることでこれ以降の章で扱う各種データを送受信します。
        * <del>sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。</del>
        * <span class="since-c">2.0</span>
        低レイヤーの送受信自体は別スレッドで行われますが、送信データをキューに入れる/受信したデータをキューから取り出して処理するのがsync()関数です。
    * データを1回送信して終了するプログラムではなく、変化するデータを繰り返し送信/受信するようなプログラムの場合は、周期実行している場所があればそこで繰り返し呼ぶようにするとよいと思います。
    ```cpp
    while(true){
        // 繰り返し実行する処理...
        wcli.sync();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ```
    * <span class="since-c">2.0</span>
    Client::loopSyncFor(), Client::loopSyncUntil() は指定した時間の間、
    また Client::loopSync() は通信を切断するまでずっと sync() を繰り返します。
        * ただしサーバーに接続しておらず autoReconnect() がオフの場合は、即座にreturnします。(デッドロック回避)
    ```cpp
    while(true){
        // 繰り返し実行する処理...
        wcli.loopSyncFor(std::chrono::milliseconds(100));
    }
    ```

    \note
    * <del>データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。</del>
    * <span class="since-c">1.2</span>
    <del>Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。</del>
    * <span class="since-c">2.0</span>
    受信した各種データの処理がsync()で行われるため、データを受信するだけの使い方の場合でもsync()は必要です。

    \warning
    * <span class="since-c">2.0</span>
    これ以降の章で扱う各種データの受信時にコールバックを設定できますが、それはsync()を呼んだスレッドで実行されます。
    そのため長時間かかるコールバックを登録した場合、その間sync()を呼んだメインスレッドがブロックされるだけでなく、他のデータの受信もできなくなるため注意してください。

    \warning
    * 1msに1回程度の頻度までは動作確認していますが、
    それ以上短い周期で sync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

- <b class="tab-title">C</b>
    ```cpp
    wcfSync(wcli);
    ```

    * wcfSync() をすることでこれ以降の章で扱う各種データを送信します。
        * <del>wcfSync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。</del>
            * <span class="since-c">2.0</span>
            低レイヤーの送受信自体は別スレッドで行われますが、送信データをキューに入れる/受信したデータをキューから取り出して処理するのがwcfSync()関数です。
    * データを1回送信して終了するプログラムではなく、変化するデータを繰り返し送信/受信するようなプログラムの場合は、周期実行している場所があればそこで繰り返し呼ぶようにするとよいと思います。
    ```c
    while(1){
        // 繰り返し実行する処理...

        wcfSync(wcli);
        usleep(100000);
    }
    ```
    * <span class="since-c">2.0</span>
    wcfLoopSyncFor(wcli, timeout), wcfLoopSyncUntil(wcli, timeout) または wcfLoopSync() は指定した時間の間(または永遠に) wcfSync() を繰り返します。
        * ただしサーバーに接続しておらず wcfAutoReconnect() がオフの場合は、即座にreturnします。(デッドロック回避)
    ```cpp
    while(1){
        // 繰り返し実行する処理...

        wcfLoopSyncFor(wcli, 100000); // 100000μs = 100ms
    }
    ```

    \note
    * <del>Funcの呼び出しとデータ受信リクエストの送信は wcfSync() とは非同期に行われるので wcfSync() は不要です。</del>
    * <span class="since-c">2.0</span>
    受信した各種データの処理がwcfSync()で行われるため、データを受信するだけの使い方の場合でもwcfSync()は必要です。

    \warning
    * <span class="since-c">2.0</span>
    これ以降の章で扱う各種データの受信時にコールバックを設定できますが、それはwcfSync()を呼んだスレッドで実行されます。
    そのため長時間かかるコールバックを登録した場合、その間wcfSync()を呼んだメインスレッドがブロックされるだけでなく、他のデータの受信もできなくなるため注意してください。

    \warning
    * 1msに1回程度の頻度までは動作確認していますが、
    それ以上短い周期で wcfSync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

- <b class="tab-title">JavaScript</b>
    ```ts
    wcli.sync();
    ```
    * sync() をすることでこれ以降の章で扱う各種データを送信します。
        * メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
    * データを1回送信して終了するプログラムではなく、変化するデータを繰り返し送信するようなプログラムの場合は、setIntervalなどを使って繰り返し呼ぶようにするとよいと思います。
    ```ts
    setInterval(() => {
        wcli.sync();
    }, 100);
    ```

    * データの受信処理は非同期で(sync()を呼ぶタイミングとは無関係に)行われます。

    \note
    * ~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~
    * <span class="since-js">1.1</span>
    Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。

- <b class="tab-title">Python</b>
    ```python
    wcli.sync();
    ```
    * sync() をすることでこれ以降の章で扱う各種データを送信します。
        * sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
    * データを1回送信して終了するプログラムではなく、変化するデータを繰り返し送信/受信するようなプログラムの場合は、周期実行している場所があればそこで繰り返し呼ぶようにするとよいと思います。
    ```python
    while True:
        # ...

        wcli.sync()
        time.sleep(0.1)
    ```

    * データの受信処理は非同期で(sync()を呼ぶタイミングとは無関係に)行われます。

</div>

## 切断する

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    wcli.close();
    ```
    * Clientのデストラクタでも自動的に Client::close() が呼ばれます。
    * <span class="since-c">2.0</span>
    Client::loopSync() はclose()で停止します。
    (別スレッドからでも、loopSync内から呼ばれたコールバックの中などでも可)

    \note
    * <span class="since-c">2.0</span>
    close()を呼んだ時点でサーバーに接続できていた場合は、sync()でキューに入れたメッセージがすべて送信したあとに切断されます。
        * waitConnection() → sync() → close() とすれば確実にデータを送信することができます。
        * start()を使ってまだ接続が完了していない場合、または1回接続した後で通信が切断された場合など、サーバーに未接続の状態でclose()が呼ばれたときはメッセージを送信することなく終了してしまいます。
    * Clientのデストラクタは通信を切断するまで待機します。
    
- <b class="tab-title">C</b>
    ```c
    wcfClose(wcli);
    ```
    * <span class="since-c">2.0</span>
    wcfLoopSync() はwcfClose()で停止します。
    (別スレッドからでも、loopSync内から呼ばれたコールバックの中などでも可)

    \note
    * <span class="since-c">2.0</span>
    wcfClose()を呼んだ時点でサーバーに接続できていた場合は、wcfSync()でキューに入れたメッセージがすべて送信したあとに切断されます。
        * wcfWaitConnection() → wcfSync() → wcfClose() とすれば確実にデータを送信することができます。
        * wcfStart()を使ってまだ接続が完了していない場合、または1回接続した後で通信が切断された場合など、サーバーに未接続の状態でwcfClose()が呼ばれたときはメッセージを送信することなく終了してしまいます。
    * wcfClose()は通信を切断するまで待機します。

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
    
- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>

    `wcfServerVersion(wcli)`, `wcfServerName(wcli)` で取得できます。

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

- <b class="tab-title">C</b>
    \since <span class="since-c">2.0</span>
    
    `wcfServerHostName(wcli)` で取得できます。

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
| [3-2. Building from Source](32_building.md) | [4-2. Member](42_member.md) |

</div>
