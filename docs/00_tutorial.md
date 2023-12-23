# Tutorial

WebCFaceの機能紹介・チュートリアルです。

## 環境構築
READMEにしたがってwebcface, webcface-webui, webcface-toolsをインストールしましょう。

## Server
WebCFaceを使用するときはserverを常時立ち上げておく必要があります。
```sh
webcface-server
```
でサーバーを起動します

* コマンドラインオプションで起動するポートを変更できたりします。詳細は`webcface-server -h`で確認してください

## WebUI
serverは起動したまま、起動時に表示されるurl (http://localhost:7530/index.html) をブラウザで開きましょう。

WebCFaceにクライアントが接続すると、WebUI右上のMenuに表示されます。
Menuから見たいデータを選ぶことで小さいウィンドウのようなものが現れデータを見ることができます。

ウィンドウの表示状態などは自動的にブラウザ(LocalStorage)に保存され、次回アクセスしたときに復元されます。

## データの送信
### value
WebCFaceではROSのTopicのようにデータを送受信することができます。

WebCFaceにデータを送信してみましょう。
```sh
webcface-send test
```
を実行し、そこにいくつか数字を打ち込んでみましょう。(1つ入力するごとに改行してください)

WebUIから「webcface-send」→「test」を選ぶと、グラフが表示されると思います。

```
$ webcface-send test
5
7
3
4
9
1
2
3
4
6
```
![tutorial_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_value.png)

### text
(起動しているwebcface-sendは終了して)
```sh
webcface-send -t text test
```
を実行し、そこに文字列を打ち込んでみましょう。
今度はWebUIから「webcface-send」→「Text Variables」を開くと入力した文字列が表示されるはずです。

```
$ webcface-send -t text test
hello, world!
```
![tutorial_text](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_text.png)

### log
```sh
webcface-send -t log
```
を実行し、そこに文字列を打ち込んでみましょう。
WebUIから「webcface-send」→「Logs」を開くと入力した文字列が表示されるはずです。

```
$ webcface-send -t log
hoge
[2023-10-19 19:40:35.603] [webcface-send] [info] hoge
fuga
[2023-10-19 19:40:37.000] [webcface-send] [info] fuga
piyo
[2023-10-19 19:40:38.447] [webcface-send] [info] piyo
```
![tutorial_logs](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_logs.png)

### view
現在コマンドラインからは使用できませんが、図のようにテキストやボタンなどの配置を指定して表示させる機能があります。

![tutorial_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

### func
コマンドラインからは利用できませんが、関数をwebcfaceに登録し他のクライアントやWebUIから呼び出すことができます。

![tutorial_func.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_func.png)

## tools
webcface-toolsには`webcface-send`の他にもWebCFaceと通信して使うプログラムがいくつかあります。
詳しくは[webcface-toolsのリポジトリ](https://github.com/na-trium-144/webcface-tools)を参照してください。

## Clientプログラムを作る

C++でWebCFaceを使う場合は、次のようにCMakeでWebCFaceクライアントのライブラリとリンクすることができます。
```cmake
find_package(webcface CONFIG REQUIRED)
target_link_libraries(target PRIVATE webcface::webcface)
```

C++のソースコードでは`<webcface/webcface.h>`をincludeしてください。
次ページ以降でC++での使い方を解説します。

[src/example/](https://github.com/na-trium-144/webcface/tree/main/src/example) にサンプルのコードがあるので参考にしてください。
exampleはserverといっしょにインストールされており、`webcface-example-main`, `webcface-example-recv`コマンドで実行してみることができます。
mainは各種データの送信、recvはmainが送信したデータの受信とmainにある関数の呼び出しをするプログラムになっています。

![c++ ver1.3](https://img.shields.io/badge/1.3~-00599c?logo=C%2B%2B)
C++ではなくCからアクセスできるAPIもあります。
[Interface for C](80_c_interface.md) を参照してください。
MATLABなど、Cのライブラリにアクセスすることができる言語からwebcfaceのライブラリをロードして使用することができます。

Python, JavaScriptのクライアントも使い方はだいたい同じです。
次ページ以降のC++での使い方を軽く読んだあとにPython、JavaScriptのリファレンスでC++との相違点を確認してください。
* [webcface-python APIリファレンス](https://na-trium-144.github.io/webcface-python/)
* [webcface-js APIリファレンス](https://na-trium-144.github.io/webcface-js/)

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
|  | [Client](01_client.md) |

</div>
