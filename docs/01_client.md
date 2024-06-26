# Client

\tableofcontents
\sa
* C++ webcface::Client (`webcface/client.h`)
* C Reference: c_wcf/client.h
* JavaScript [Client](https://na-trium-144.github.io/webcface-js/classes/Client.html)
* Python [webcface.Client](https://na-trium-144.github.io/webcface-python/webcface.client.html#webcface.client.Client)

WebCFaceのメインとなるクラスです。

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

    \note
    * `start()` の代わりに `waitConnection()` を使うと接続が完了するまで待機することができます。
    * 接続できているかどうかは `wcli.connected()` で取得できます。
    * 通信が切断された場合は自動で再接続します。
        * <span class="since-c">1.11.1</span>
    `autoReconnect(false)`をすると自動で再接続しなくなります。
    その場合`waitConnection()`は1度だけ接続を試行し失敗してもreturnするという挙動になります。
    * <span class="since-c">1.11</span>
    接続先のアドレスが`127.0.0.1`の場合はTCPポートと別にUnixドメインソケットへの接続も試行し、
    接続できた場合はUnixドメインソケットの接続を優先して使用します。
        * WSL1上のクライアントは `/mnt/c/ProgramData/webcface/ポート番号.sock` への接続も試行します。これによりWSL1とWindowsの間ではどちらでサーバーを建ててもどちらのクライアントからも相互に接続できます。
    * <span class="since-c">1.11</span>
    WSL2ではUnixドメインソケットを使った相互接続はできませんが、WSL2のクライアントは接続先のアドレスが`127.0.0.1`の場合に限りホストのWindowsのIPアドレスのTCPポートへも接続を試行します。これによりWSL2とWindowsの間でも相互に通信が可能です。
    * Clientはstart()時に2つのスレッド(std::thread)を建てます。
    1つはWebSocketの送受信処理用で、送信用キューにあるメッセージを送信し、受信したら受信用キューに入れます。
    もう1つは受信したメッセージをパースしてコールバックを呼んだりといった処理をします。
        * Callメッセージ(Func呼び出しのメッセージ)を受信したときと、自分自身のFuncに対してrunAsync()を呼び出したときは、その呼び出し1回ごとに新しいスレッドを建てその中で関数を実行します。

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

    \note
    * <span class="since-c">2.0</span>
    ワイド文字列を使用したい場合はそれぞれ `wcfInitDefaultW()`, `wcfInitW()`
    (詳細はこのページのEncodingの章を参照)
    * 接続できているかどうかは `wcfIsConnected(wcli)` で取得できます。
    * 通信が切断された場合は自動で再接続します。
    * <span class="since-c">1.11</span>
    接続先のアドレスが`127.0.0.1`の場合(wcfInitDefaultを使用した場合も含む)はTCPポートと別にUnixドメインソケットへの接続も試行し、
    接続できた場合はUnixドメインソケットの接続を優先して使用します。
        * WSL1上のクライアントは /mnt/c/ProgramData/webcface/ポート番号.sock への接続も試行します。これによりWSL1とWindowsの間ではどちらでサーバーを建ててもどちらのクライアントからも相互に接続できます。
    * <span class="since-c">1.11</span>
    WSL2ではUnixドメインソケットを使った相互接続はできませんが、WSL2のクライアントは接続先のアドレスが`127.0.0.1`の場合に限りホストのWindowsのIPアドレスのTCPポートへも接続を試行します。これによりWSL2とWindowsの間でも相互に通信が可能です。
    * ClientはwcfStart()時に2つのスレッド(std::thread)を建てます。
    1つはWebSocketの送受信処理用で、送信用キューにあるメッセージを送信し、受信したら受信用キューに入れます。
    もう1つは受信したメッセージをパースしてコールバックを呼んだりといった処理をします。
        * Callメッセージ(Func呼び出しのメッセージ)を受信したときと、自分自身のFuncに対してwcfFuncRunAsync()を呼び出したときは、その呼び出し1回ごとに新しいスレッドを建てその中で関数を実行します。

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

webcfaceのAPIではほぼすべての関数でマルチバイト文字列(`std::string`, `const char *`)の代わりにワイド文字列(`std::wstring`, `const wchar_t *`)が使用可能です。

マルチバイト文字列はデフォルトではWindows,Linux,MacOSともにUTF-8エンコーディングとして扱われます。
ただしWindowsでのみClientの初期化前に最初に C++: webcface::Encoding::usingUTF8(), C: wcfUsingUTF8() を false に設定することでANSIエンコーディング(日本語環境ではShiftJIS)として扱われるようにすることもできます。
(webcface内部でUTF-8との変換が行われます)

ワイド文字列はWindowsではUTF-16、Linux/MacOSではUTF-32でエンコードされていることを想定しています。
Windowsでは、ワイド文字列を使用する場合の usingUTF8 の設定は
Client::loggerSink(), logger() を使用する場合はtrueに、
Client::loggerWStreamBuf(), loggerWOStream() を使用する場合は出力するコンソールのコードページに合わせてください。
(詳細は[log](./40_log.md)のページを参照)

C++で文字列を返すAPI、およびCのAPI全般ではワイド文字列を使用する関数やstruct名には末尾に`W`が付きます。

## sync

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    while(true){
        // ...

        wcli.sync();
        std::this_thread.sleep_for(std::chrono::milliseconds(100));
    }
    ```
- <b class="tab-title">C</b>
    ```c
    while(1){
        // ...
        wcfSync(wcli);
    }
    ```

- <b class="tab-title">JavaScript</b>
    ```ts
    setInterval(() => {
        wcli.sync();
    }, 100);
    ```
- <b class="tab-title">Python</b>
    ```python
    while True:
        # ...

        wcli.sync()
        time.sleep(0.1)
    ```

</div>

sync() をすることでこれ以降の章で扱う各種データを実際に送信します。

sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。

~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~  
<span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。

\warning
1msに1回程度の頻度までは動作確認していますが、
それ以上短い周期で sync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

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


<div class="tabbed">

- <b class="tab-title">C++</b>
    `wcli.serverVersion()`, `wcli.serverName()` でサーバーの情報を取得できます。
- <b class="tab-title">JavaScript</b>
    `wcli.serverVersion`, `wcli.serverName` でサーバーの情報を取得できます。
- <b class="tab-title">Python</b>
    `wcli.serverVersion`, `wcli.serverName` でサーバーの情報を取得できます。
    
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

\note
<span class="since-js">1.1</span>
ClientのコンストラクタでlogLevelに `"trace"` または `"verbose"` を指定することでも表示できます。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Overview](00_overview.md) | [Member](02_member.md) |

</div>
