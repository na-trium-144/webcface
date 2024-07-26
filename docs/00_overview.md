# Overview

\tableofcontents

WebCFaceの機能紹介・チュートリアルです。

このドキュメントでは仕様変更された機能は
C,C++のwebcface <span class="since-c"></span>,
webcface-js <span class="since-js"></span>,
webcface-python <span class="since-py"></span>
の画像で示します。

## 環境構築
READMEにしたがって webcface, webcface-webui, webcface-tools をインストールしましょう。

さらにJavaScriptで利用したい場合は `npm install webcface` 、
Pythonで利用したい場合は `pip install webcface` でクライアントをインストールしてください。

## Server
WebCFaceを使用するときはserverを常時立ち上げておく必要があります。

### コマンドラインから
```sh
webcface-server
```
でサーバーを起動します。
デフォルトでは7530番ポートを開きクライアントの接続を待ちます。

* コマンドラインオプションで起動するポートを変更できたりします。詳細は`webcface-server -h`で確認してください

<span class="since-c">1.11</span>
TCPポートだけでなく、Unixドメインソケットも開くようになりました。
Linux,Macでは `/tmp/webcface/ポート番号.sock`
Windowsでは `C:\ProgramData\webcface\ポート番号.sock`
が自動的に作成されます。

さらに、WSL1上で起動した場合は `/mnt/c/ProgramData/webcface/ポート番号.sock` も追加で開きます。
これによりWindows側のクライアントがWSL1側のサーバーに接続することができます。

\note
標準エラー出力にログが出力されます。
デフォルトではクライアントの接続と切断がログに表示されます。  
<span class="since-c">1.1.7</span>
コマンドライン引数で `-v` を渡すとクライアントから送られてきたメッセージ、
`-vv` を渡すと送受信したメッセージすべてをログに表示します。
(ログが長く見づらくなります)

### サービスとして (Linuxのみ)

\since <span class="since-c">1.5.3</span>

配布しているdebパッケージでは [cmake/webcface-server.service](https://github.com/na-trium-144/webcface/blob/main/cmake/webcface-server.service) がインストールされ、
```sh
sudo systemctl enable webcface-server
sudo systemctl start webcface-server
```
でサーバーを常時自動起動させることができます。

## WebUI
serverは起動したまま、起動時に表示されるurl (http://IPアドレス:7530/index.html) をブラウザで開きましょう。

WebCFaceにクライアントが接続すると、WebUI右上のMenuに表示されます。
Menuから見たいデータを選ぶことで小さいウィンドウのようなものが現れデータを見ることができます。

ウィンドウの表示状態などは自動的にブラウザ(LocalStorage)に保存され、次回アクセスしたときに復元されます。

### WebUI Desktop

ver2は準備中

<details><summary>WebCFace ver1 (WebUI ver1.6.0)まで (WebUI Server Mode)</summary>

WebUIをブラウザーからではなくアプリとして開くと、バックグラウンドでいっしょにサーバーが起動します。

* Windowsではスタートメニューの WebCFace → WebCFace WebUI Server を起動してください。
* MacOSではREADMEにしたがってAppバンドルをダウンロードして起動してください

\note ソースコードとバイナリ配布はこのリポジトリではなく [webcface-webui](https://github.com/na-trium-144/webcface-webui) に含まれるので、ソースからビルドする場合または個別にダウンロードしたい場合はそちらを参照してください

この場合、通常のWebUIにはないメニュー項目がいくつか現れます。
ここで操作した内容は自動的に保存されます。

* Import Config, Export Config: 自動保存されるServer Modeの設定を別ファイルに保存したり読み込むことができます。
* Server Status: サーバーの状態、IPアドレスが見れます。またLauncherの起動、停止ができます。
* Logs: サーバーの出力するログが見れます。
* Launcher Config: [webcface-launcher](./70_launcher.md) の設定を編集できます。

![webui-server](https://github.com/na-trium-144/webcface/raw/main/docs/images/webui-server.png)

WebUIの画面を閉じるとserverも終了します。
次に画面を開いた時自動的に前回の状態が復元されます。

\note
設定を自動保存する場所はWindowsでは `C:\Users\(user)\AppData\Roaming\webcface\sg.toml` 、それ以外では `$HOME/.webcface.sg.toml` です。

</details>

## PlotJuggler

[plotjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin) のREADMEにしたがってプラグインをビルドしてインストールすると、
[PlotJuggler](https://github.com/facontidavide/PlotJuggler) からWebCFaceのデータにアクセスできるようになります。
ビルド済みバイナリでの配布はしていません。

## Clientライブラリ

### C++

C++でWebCFaceを使う場合は、次のようにCMakeでWebCFaceクライアントのライブラリとリンクすることができます。
```cmake
find_package(webcface CONFIG REQUIRED)
target_link_libraries(target PRIVATE webcface::webcface)
```

* find_packageでwebcfaceが見つからない場合はwebcfaceのインストール場所を`CMAKE_PREFIX_PATH`か`webcface_DIR`に設定してください。
* FetchContentやsubmoduleでこのリポジトリを追加して使うこともできます。また、ROS2のワークスペースのsrcに追加してcolconでビルドすることもできます。
    * CMakeのオプション(OpenCVを使用しないようにする、staticライブラリにするなど)はREADMEを参照してください

<details><summary>CMakeを使わない場合は</summary>

* Windows (MSVC)
    * インクルードディレクトリ: `C:\Program Files\WebCFace\include`, `C:\Program Files\WebCFace\opencv\include`
    * ライブラリディレクトリ: `C:\Program Files\WebCFace\lib`, `C:\Program Files\WebCFace\opencv\x64\vc16\lib`
    * リンクするライブラリは
        * Releaseの場合 webcface10.lib, spdlog.lib, opencv_world490.lib
        * Debugの場合 webcface10d.lib, spdlogd.lib, opencv_world490d.lib
    * また、`C:\Program Files\WebCFace\bin` を環境変数のPathに追加するか、その中にあるdllファイルを実行ファイルのディレクトリにコピーして読み込ませてください
* Linux
    * pkgconfigを使用してコンパイル時の引数に `$(pkg-config --cflags webcface)` 、リンク時に `$(pkg-config --libs webcface)` を渡せばよいです
    * 手動でリンクするなら lib/libwebcface.so をリンクしてください
        * Releasesで配布しているdebパッケージの場合はインストール先は /usr です
* MacOS
    * Linuxと同様pkgconfigを使ってコンパイル、リンクできると思います

</details>

C++のソースコードでは`<webcface/webcface.h>`をincludeするとwebcfaceのすべての機能が使用できます。
次ページ以降でC++での使い方を解説します。

<span class="since-c">1.10</span>
`<webcface/client.h>`, `<webcface/value.h>`など必要なヘッダファイルだけincludeして使うこともでき、コンパイル時間を短縮できます。

### C

\since <span class="since-c">1.5</span>

C++ではなくCからアクセスできるAPIとして、wcf〜 で始まる名前の関数やstructが用意されています。
(C++ライブラリのうちの一部の機能しかまだ実装していませんが)

~~&lt;webcface/c_wcf.h&gt; をincludeすることで使えます。~~  
<span class="since-c">1.7</span> &lt;webcface/wcf.h&gt; をincludeすることで使えます。(c_wcf.hも一応使えます)  
ほとんどの関数は戻り値が <del>int 型</del> <span class="since-c">2.0</span> enum wcfStatus 型で、
成功した場合 0 (= <del>WCF_OK</del> <span class="since-c">2.0</span> WCF_OK)、例外が発生した場合正の値を返します。

MATLABなど、Cのライブラリにアクセスすることができる言語からwebcfaceのライブラリをロードして使用することができます。

~~CMakeの書き方はC++の場合と同じです。~~  
<span class="since-c">1.5.1</span>
Cの場合は `webcface::wcf` をリンクしてください。

CMakeを使わない場合リンクするライブラリはC++の場合と同じです。

### その他

Python, JavaScriptのクライアントも使い方はだいたい同じです。
次ページ以降のドキュメントとともにPython、JavaScriptのリファレンスも参照してください。
* [webcface-python APIリファレンス](https://na-trium-144.github.io/webcface-python/)
* [webcface-js APIリファレンス](https://na-trium-144.github.io/webcface-js/)

## Tools

webcface-toolsにはWebCFaceと通信して使うコマンドラインツールがいくつかあります。

それぞれ以下のページを参照してください。
* [webcface-launcher](./70_launcher.md)
* [webcface-send](./71_send.md)
* [webcface-cv-capture](./72_cv_capture.md)

## データ型
WebCFaceではROSのTopicのようにデータを送受信することができます。

### Value
数値データ、または1次元数値配列を送受信する型です。
double型で送受信されます。

WebUI では受信したデータがグラフとして表示されます。
(グラフの画面を表示するより前のデータにはアクセスできません)

マウスドラッグで移動、Ctrl+スクロール・Ctrl+Alt+スクロールでそれぞれx, y方向の拡大縮小ができます。

![tutorial_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_value.png)

\warning WebUI では数値配列のデータの表示が未実装です (配列の先頭の値のみが表示されます)

### Text

文字列データを送受信する型です。

WebUI では図のように文字列が表示されます。

![tutorial_text](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_text.png)

### View
図のようにテキストやボタンなどの配置を指定してWebUIに表示させる機能です。

![tutorial_view.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

### Image
画像データを送受信する型です。
C++ではOpenCVの画像データと相互変換できます。

画像のリサイズ、圧縮などの処理をサーバー側でやる機能があります

WebUIでは最大10fpsで画像を表示できます。

### Canvas2D
2Dの図形の描画データを送受信する型です。

WebUI上で2次元の図形を描画することができます。

また、Viewのように特定の部分をクリックしたときに関数を実行させることもできUIとしても使えます。

![tutorial_canvas2d.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_canvas2d.png)

WebUIではマウス操作やタッチ操作で画面を動かしたり拡大縮小できます。
右下のアイコンから操作説明が見れます。

### Canvas3D
3D空間上のオブジェクト配置データを送受信する型です。

WebUI上で3次元の図形を描画することができます。

![tutorial_canvas3d.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_canvas3d.png)

WebUIではマウス操作やタッチ操作で画面を動かしたり拡大縮小できます。
右下のアイコンから操作説明が見れます。

### RobotModel

ロボットのリンクと関節の構造の情報を送受信する型です。
事前に定義したRobotModelの各関節の角度だけを後で指定してCanvas3Dに描画することができます。

WebUI上ではCanvas3D上に描画して表示する他、RobotModel単体でも表示することができます。

![tutorial_wheel.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_wheel.png)

### Func
関数をwebcfaceに登録し、他のクライアントやWebUIから呼び出すことができます。

![tutorial_func.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_func.png)

### Log
エラーメッセージなどを送受信するデータ型です。
出力した文字列1行ごとにタイムスタンプとログのレベルの情報を加えて送信し、WebUIに表示することができます。

![tutorial_logs](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_logs.png)

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
|  | [Client](01_client.md) |

</div>
