# 2-2. WebUI

\tableofcontents

WebCFaceはWebCFaceの通信データにアクセスできるGUIです。

## Serverから

webcface-server を起動し、コンソールに表示されるurl (http://127.0.0.1:7530/index.html など) をブラウザで開きましょう。

WebUI右上のMenuにWebCFaceに接続しているクライアントの一覧が表示されます。
Menuから見たいデータを選ぶことで小さいウィンドウのようなものが現れデータを見ることができます。

\note
ウィンドウの表示状態などは自動的にブラウザ(LocalStorage)に保存され、次回アクセスしたときに復元されます。

## WebUI Desktop

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
* Launcher Config: [webcface-launcher](./71_launcher.md) の設定を編集できます。

![webui-server](https://github.com/na-trium-144/webcface/raw/main/docs/images/webui-server.png)

WebUIの画面を閉じるとserverも終了します。
次に画面を開いた時自動的に前回の状態が復元されます。

\note
設定を自動保存する場所はWindowsでは `C:\Users\(user)\AppData\Roaming\webcface\sg.toml` 、それ以外では `$HOME/.webcface.sg.toml` です。

</details>

<!--
    ## PlotJuggler

[plotjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin) のREADMEにしたがってプラグインをビルドしてインストールすると、
[PlotJuggler](https://github.com/facontidavide/PlotJuggler) からWebCFaceのデータにアクセスできるようになります。
ビルド済みバイナリでの配布はしていません。

-->

## 各種データ型へのアクセス

### Value

数値データ、または1次元数値配列を送受信する型です。

WebUI では受信したデータがグラフとして表示されます。
(グラフの画面を表示させた時点より前のデータにはアクセスできません)

<del>マウスドラッグで移動、Ctrl+スクロール・Ctrl+Alt+スクロールでそれぞれx, y方向の拡大縮小ができます。</del>  
(ver1.7から)右下の手のアイコンをクリックすると、移動・ズームができる状態になり、

* (マウス)ドラッグ / (タッチ)スライド で移動
* (マウス)スクロール で Y 方向拡大縮小
* Ctrl(Command⌘)+スクロール で X 方向拡大縮小
* (タッチ)2本指操作 で拡大縮小

ができます。
右下の ? アイコンからも操作説明が見れます。

また、下のスライダーでも時間をスクロールできます。

(↓スクショは過去のバージョン)

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
| [2-1. Server](21_server.md) | [3-1. Setup WebCFace Library](31_setup.md) |

</div>
