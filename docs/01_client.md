# Client

API Reference →
C++ webcface::Client
JavaScript [Client](https://na-trium-144.github.io/webcface-js/classes/Client.html)
Python [webcface.Client](https://na-trium-144.github.io/webcface-python/webcface.client.html#webcface.client.Client)

WebCFaceのメインとなるクラスです。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    #include <webcface/webcface.h>

    webcface::Client wcli("sample");

    wcli.start();
    // または wcli.waitConnection();
    ```
- <b class="tab-title">JavaScript</b>
    ```ts
    import { Client } from "webcface";

    const wcli = Client("sample");

    wcli.start();
    ```
- <b class="tab-title">Python</b>
    ```python
    import webcface

    wcli = webcface.Client("sample")

    wcli.start()
    # または wcli.wait_connection()
    ```

</div>

* ~~Client オブジェクトを作ると別スレッドでサーバーへの接続を開始します。~~
    * ![c++ ver1.2](https://img.shields.io/badge/1.2~-00599c?logo=C%2B%2B)
![js ver1.1](https://img.shields.io/badge/1.1~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
Client オブジェクトを作り、start() を呼ぶことでサーバーへの接続を開始します。
* バックグラウンド(別スレッド)で接続が完了するまでの間はデータの受信などはできません。
    * (C++, Python のみ) start() の代わりに waitConnection() を使うと接続が完了するまで待機することができます。
* 通信が切断された場合は自動で再接続します。
* コンストラクタに指定するのはこのクライアントの名前(Memberの名前)です。
    * 同時に接続している他のクライアントと名前が被らないようにしてください。
    * コンストラクタに名前を指定しない、または空文字列の場合、読み取り専用モードになります。
この場合他のクライアントとの被りは問題ありませんがデータを送信することができなくなります。
* デフォルトではlocalhostの7530ポートに接続を試みますが、サーバーのポートを変更していたり別のpcに接続する場合はコンストラクタの引数でサーバーのアドレスとポートを指定できます。

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

Client::sync() をすることでこれ以降の章で扱う各種データを実際に送信します。

* sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。
* ~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~
    * ![c++ ver1.2](https://img.shields.io/badge/1.2~-00599c?logo=C%2B%2B)
![js ver1.1](https://img.shields.io/badge/1.1~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。
* 1msに1回程度の頻度までは動作確認しています。
    * それ以上短い周期で sync() を呼ぶとサーバーの処理が間に合わなくなるかもしれません(未検証)

## 切断する

Client::close() で切断します。

* C++ではClientのデストラクタでも自動的に切断します。

## ログ出力

![c++ ver1.1.7](https://img.shields.io/badge/1.1.7~-00599c?logo=C%2B%2B)
![js ver1.1](https://img.shields.io/badge/1.1~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
`WEBCFACE_VERBOSE` 環境変数が存在する場合、WebCFaceの通信に関するログ(接続、切断、メッセージのエラー)が出力されます。
また `WEBCFACE_TRACE` 環境変数が存在すると内部で使用しているlibcurlの出力も表示します。

* ![js ver1.1](https://img.shields.io/badge/1.1~-f7df1e?logo=JavaScript&logoColor=black)
ClientのコンストラクタでlogLevelに `"trace"` または `"verbose"` を指定することでも表示できます。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Overview](00_overview.md) | [Member](02_member.md) |

</div>
