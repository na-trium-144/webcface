# Tutorial

WebCFaceの機能紹介・チュートリアルです。

### 環境構築
READMEにしたがってwebcface, webcface-webui, webcface-toolsをインストールしましょう。

### Server
WebCFaceを使用するときはserverを常時立ち上げておく必要があります。
```sh
webcface-server
```
でサーバーを起動します

### WebUI
serverは起動したまま、起動時に表示されるurl (http://localhost:7530/) をブラウザで開きましょう。

WebCFaceにクライアントが接続すると、WebUI右上のMenuに表示されます。
Menuから見たいデータを選ぶことで小さいウィンドウのようなものが現れデータを見ることができます。

ウィンドウの表示状態などは自動的にブラウザ(LocalStorage)に保存され、次回アクセスしたときに復元されます。

### データの送信
WebCFaceではROSのTopicのようにデータを送受信することができます。

WebCFaceにデータを送信してみましょう。
```sh
webcface-send test
```
を実行し、そこにいくつか数字を打ち込んでみましょう。(1つ入力するごとに改行してください)

WebUIから「webcface-send」→「test」を選ぶと、グラフが表示されると思います。

また、(起動しているwebcface-sendは終了して)
```sh
webcface-send --text test
```
を実行し、そこに文字列を打ち込んでみましょう。
今度はWebUIから「webcface-send」→「Text Variables」を開くと入力した文字列が表示されるはずです。


### Client

```cmake
find_package(webcface)
target_link_libraries(target PRIVATE webcface::webcface)
```

```cpp
#include <webcface/webcface.h>

WebCFace::Client wcli("name of this client program");
```
