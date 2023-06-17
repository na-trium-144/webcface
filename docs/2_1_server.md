# サーバー設定

see also WebCFace::Server

c++のヘッダーは`<webcface/server.hpp>`

## Start

プログラムの初期化処理として`WebCFace::startServer();` (python → `webcface.start_server()`)以下を1回実行します

プログラムを実行すると
```
[WebCFace] start server url= http://192.168.3.4:3001/
```
のように表示されます。
このurlをブラウザで開くと、フロントエンドが開きます

## Mutex (C++のみ)

(任意)プログラムのメインループ内で WebCFace::callback_mutex をロックしてください

例
```cpp
std::lock_guard lock(WebCFace::callback_mutex);
```

## SendData

`WebCFace::sendData();`(python → `webcface.send_data()`)を1制御周期に1回実行されるようにしてください

## サーバー名
サーバー名を設定するとフロントエンドに表示されます(任意)

`WebCFace::setServerName` (python → `webcface.set_server_name()`)

## 関連サーバーの設定
WebCFaceを使った複数のバックエンドに1つのフロントエンドから接続したい場合、
別のバックエンドのアドレスとポートを設定すると、フロントエンドが自動的にそのサーバーにも接続するようになります。

`WebCFace::addRelatedServer` (python → `webcface.add_related_server()`)

この場合サーバー名設定をすることを推奨します。
