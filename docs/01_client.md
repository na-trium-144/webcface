# Client

API Reference → webcface::Client

WebCFaceのメインとなるクラスです。

まずはClientのオブジェクトを作成しましょう。
引数には一意な名前を設定してください。

```cpp
#include <webcface/webcface.h>

webcface::Client wcli("sample");
// または webcface::Client wcli;

wcli.start();
// または wcli.waitConnection();
```

~~Client オブジェクトを作ると別スレッドでサーバーへの接続を開始します。~~  
![ver1.2から](https://img.shields.io/badge/ver1.2~-00599c?logo=C%2B%2B)
Client オブジェクトを作り、start() または waitConnection() を呼ぶことで別スレッドでサーバーへの接続を開始します。
start() はブロックしませんが waitConnection() は接続が完了するまで待機します。

通信が切断された場合は自動で再接続します。

コンストラクタに指定するのはこのクライアントの名前(Memberの名前)です。
同時に接続している他のクライアントと名前が被らないようにしてください。  
コンストラクタに名前を指定しない、または空文字列の場合、読み取り専用モードになります。
この場合他のクライアントとの被りは問題ありませんがデータを送信することができなくなります。

デフォルトではlocalhostの7530ポートに接続を試みますが、サーバーのポートを変更していたり別のpcに接続する場合はコンストラクタの引数でサーバーのアドレスとポートを指定できます。

プログラムを起動できたら、WebUIを開いてみましょう。
そして右上のメニューを開き、Clientの初期化時に指定した名前がそこに表示されていれば正しく通信できています。

## sync

```cpp
while(true){
    // ...

    wcli.sync();
}
```

Client::sync() をすることでこれ以降の章で扱う各種データを実際に送信します。
sync()自体は送信処理はせずキューに入れるだけであり、ノンブロッキングです。
メインプログラムの周期実行される場所などで繰り返し呼ぶようにしてください。

~~データを受信するだけの場合もサーバーにデータのリクエストをするためsync()が必要になります。~~  
![ver1.2から](https://img.shields.io/badge/ver1.2~-00599c?logo=C%2B%2B)
Funcの呼び出しとデータ受信リクエストの送信は sync() とは非同期に行われるので sync() は不要です。

## 切断する

Client::close() で切断します。
またはデストラクタでも自動的に切断します。

## ログ出力

![ver1.1.7から](https://img.shields.io/badge/ver1.1.7~-00599c?logo=C%2B%2B)
`WEBCFACE_VERBOSE` 環境変数が存在する場合、WebCFaceの通信に関するログ(接続、切断、メッセージのエラー)が出力されます。
また `WEBCFACE_TRACE` 環境変数が存在すると内部で使用しているlibcurlの出力も表示します。

[Tutorial](./00_tutorial.md) ←前 | 次→ [Member](./02_member.md)
