# Client

\tableofcontents
\sa
* C++ webcface::Client (`webcface/client.h`)
* C Reference: c_wcf/client.h
* JavaScript [Client](https://na-trium-144.github.io/webcface-js/classes/Client.html)
* Python [webcface.Client](https://na-trium-144.github.io/webcface-python/webcface.client.html#webcface.client.Client)

WebCFaceのメインとなるクラスです。

* ~~Client オブジェクトを作ると別スレッドでサーバーへの接続を開始します。~~
    * <span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
Client オブジェクトを作り、start() を呼ぶことでサーバーへの接続を開始します。
        * (C++, Python のみ) start() の代わりに waitConnection() を使うと接続が完了するまで待機することができます。
* バックグラウンド(別スレッド)で接続が完了するまでの間はデータの受信などはできません。
* 通信が切断された場合は自動で再接続します。
* コンストラクタに指定するのはこのクライアントの名前(Memberの名前)です。
    * 同時に接続している他のクライアントと名前が被らないようにしてください。同じ名前で複数のクライアントが接続した場合正常に通信できない場合があります。
* デフォルトではlocalhostの7530ポートに接続を試みますが、サーバーのポートを変更していたり別のpcに接続する場合はコンストラクタの引数でサーバーのアドレスとポートを指定できます。
    * 特にwebブラウザ上でJavaScriptから接続しようとする場合、サーバー側からみたlocalhostではなくブラウザ側からみたlocalhostへ接続しようとするので注意してください ([location.host](https://developer.mozilla.org/ja/docs/Web/API/Location/host) などを接続先アドレスに指定する必要があります)

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    #include <webcface/client.h>
    // または #include <webcface/webcface.h> (すべての機能をinclude)

    webcface::Client wcli("sample");
    // アドレスを指定する場合
    // webcface::Client wcli("sample", "192.168.1.1", 7530);

    wcli.start();
    // または wcli.waitConnection();
    ```
    接続できているかどうかは `wcli.connected()` で取得できます。

- <b class="tab-title">C</b>
    ```c
    #include <webcface/wcf.h>

    wcfClient *wcli = wcfInitDefault("sample");
    wcfStart(wcli);
    ```
    でclientを生成し、接続します。

    \note <span class="since-c">1.7</span> 引数にnullptrを渡した場合空文字列(後述)と同様になります

    サーバーのアドレスとポートを指定したい場合`wcfInit()`を使います
    ```c
    #include <webcface/wcf.h>

    wcfClient *wcli = wcfInit("sample", "192.168.1.1", 7530);
    wcfStart(wcli);
    ```
    接続できているかどうかは `wcfIsConnected(wcli)` で取得できます。
    
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Client } from "webcface";

    const wcli = Client("sample");
    // アドレスを指定する場合
    // const wcli = Client("sample", "192.168.1.1", 7530);

    wcli.start();
    ```
    接続できているかどうかは `wcli.connected` で取得できます。
- <b class="tab-title">Python</b>
    ```python
    import webcface

    wcli = webcface.Client("sample")
    # アドレスを指定する場合
    # wcli = webcface.Client("sample", "192.168.1.1", 7530)

    wcli.start()
    # または wcli.wait_connection()
    ```
    接続できているかどうかは `wcli.connected` で取得できます。

</div>

\note
コンストラクタに名前を指定しない、または空文字列の場合、読み取り専用モードになります。  
この場合データを送信することができなくなりますが、同じプログラムを複数起動することができます
(WebUIではこの仕様を使っています)

クライアントが正常に接続できると、サーバーのログに
```
[info] Successfully connected and initialized.
```
などと表示されます。
(クライアントの名前を指定しなかった場合は表示されません)

プログラムを起動できたら、WebUIを開いてみましょう。
そして右上のメニューを開き、Clientの初期化時に指定した名前がそこに表示されていれば正しく通信できています。

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

* sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
* ~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~
    * <span class="since-c">1.2</span>
<span class="since-js">1.1</span>
<span class="since-py"></span>
Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。
* 1msに1回程度の頻度までは動作確認しています。
    * それ以上短い周期で sync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

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
