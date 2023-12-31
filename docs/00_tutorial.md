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

## Clientライブラリ

C++でWebCFaceを使う場合は、次のようにCMakeでWebCFaceクライアントのライブラリとリンクすることができます。
```cmake
find_package(webcface CONFIG REQUIRED)
target_link_libraries(target PRIVATE webcface::webcface)
```

* find_packageでwebcfaceが見つからない場合はwebcfaceのインストール場所をCMAKE_PREFIX_PATHに設定してください。
* FetchContentやsubmoduleでこのリポジトリを追加して使うこともできます。また、ROS2のワークスペースのsrcに追加してcolconでビルドすることもできます。
    * OpenCVを使用しない場合は`WEBCFACE_USE_OPENCV`をoffにすれば使用しないようにできます。

VisualStudioでCMakeを使わない場合は  
インクルードディレクトリ: `C:\Program Files\webcface\include`, `C:\Program Files\webcface\opencv\include`  
ライブラリディレクトリ: `C:\Program Files\webcface\lib`, `C:\Program Files\webcface\opencv\x64\vc16\lib`  
を追加し、  
Releaseの場合 webcface.lib, spdlog.lib, opencv_world481.lib  
Debugの場合 webcfaced.lib, spdlogd.lib, opencv_world481d.lib  
(opencvのバージョンが異なったら読み替えてください)をリンクしてください。  
また、`C:\Program Files\webcface\bin` を環境変数のPathに追加するか、その中にあるdllファイルを実行ファイルのディレクトリにコピーして読み込ませてください


C++のソースコードでは`<webcface/webcface.h>`をincludeしてください。
次ページ以降でC++での使い方を解説します。

[src/example/](https://github.com/na-trium-144/webcface/tree/main/src/example) にサンプルのコードがあるので参考にしてください。
exampleはserverといっしょにインストールされており、`webcface-example-main`, `webcface-example-recv`コマンドで実行してみることができます。
mainは各種データの送信、recvはmainが送信したデータの受信とmainにある関数の呼び出しをするプログラムになっています。

Python, JavaScriptのクライアントも使い方はだいたい同じです。
次ページ以降のC++での使い方を軽く読んだあとにPython、JavaScriptのリファレンスでC++との相違点を確認してください。
* [webcface-python APIリファレンス](https://na-trium-144.github.io/webcface-python/)
* [webcface-js APIリファレンス](https://na-trium-144.github.io/webcface-js/)

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
|  | [Client](01_client.md) |

</div>
