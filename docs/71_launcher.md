# 7-1. webcface-launcher

\tableofcontents
\since tools ver1.0

設定ファイルにしたがってコマンドの実行、停止をするボタンをWebUIに表示します。

tomlファイルに設定を記述し、
```sh
webcface-launcher ./webcface-launcher.toml
```
のように指定して起動します。

WebUI上ではそれぞれの設定ファイルに書かれたそれぞれのコマンドを開始するボタンと停止するボタンが表示されます。

![launcher.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/launcher.png)

\note
* プロセスの起動と停止には外部ライブラリの [tiny-process-library](https://gitlab.com/eidheim/tiny-process-library/) を使用しています
* (tools ver1.4.2から、Linux,MacOS) webcface-launcherを停止すると、実行中のコマンドにもシグナルが送られます。
SIGINT(Ctrl+C)で停止しない場合は、複数回Ctrl+Cを押すとSIGTERM、SIGKILLに移行して強制的に停止します。

<span></span>

* (tools ver1.4.3から) Startボタン、Stopボタンの動作はViewを経由せずにFuncで呼び出すこともできます。
(WebUIのFunctionsの画面からも確認できます)
    * Startボタンは `member("webcface-launcher").func("コマンド名/start").runAsync()`
    * Stopボタンは `member("webcface-launcher").func("コマンド名/stop").runAsync()`  
    などとすると呼び出すことができます。
        * (tools ver2.1から) stop関数はコマンドが停止するまで完了しません。
    * (tools ver2.1から) `func("コマンド名/run")` を呼び出すと、startと同様コマンドを開始し、終了するまで待機することができます。
* (tools ver2.1から) `member("webcface-launcher").value("コマンド名.running")` でコマンドが実行中かどうかを取得できます。
    * また、 `member("webcface-launcher").value("コマンド名.exit_status")` で終了コードも取得できます。
* (tools ver2.1から) `member("webcface-launcher").log("コマンド名")` でログを取得できます。
(設定ファイルの stdout_capture の説明を参照)

## サービスとして (Linuxのみ)
配布しているdebパッケージからインストールした場合は、serverと同様に
```sh
sudo systemctl enable webcface-launcher
sudo systemctl start webcface-launcher
```
でwebcface-launcherをサービスとして自動起動することができます。
ただしその場合は設定ファイルを /etc/webcface/webcface-launcher.toml として置いてください。

## コマンドライン引数
```
Usage: webcface-launcher [OPTIONS] [config_path]
```
* config_path: 引数に設定ファイルのパスを渡してください。省略した場合はカレントディレクトリの`webcface-launcher.toml`を開きます。
* `-h`: ヘルプを表示します。
* `-a address`:  (tools ver1.1から) 接続するサーバーのアドレスです。省略時は設定ファイル内の記述を参照し、それもなければ127.0.0.1になります。
* `-p port`:  (tools ver1.1から) 接続するサーバーのポートです。省略時は設定ファイル内の記述を参照し、それもなければ7530になります。
* `-m name`:  (tools ver1.1から) WebCFaceでのメンバー名(WebUIで表示される名前)です。省略時は設定ファイル内の記述を参照し、それもなければ webcface-launcher になります。
* `-s`: (tools ver1.2から) 設定ファイルを読み込む代わりに、標準入力から入力された文字列をtomlとしてパースします。

## 設定ファイル
tomlファイルの例

```toml
[init]
address = "127.0.0.1"
port = 7530
name = "webcface-launcher"
 
[[command]]
name = "sleep"
exec = "sleep 1"
 
[[command]]
name = "main"
workdir = "/path/to/somewhere"
exec = "./main"
stdout_capture = false
stdout_utf8 = true
stop = 2
# または、
# [command.stop]
# exec = "pkill ..."
# workdir = "..."
[command.env]
FOO = "foo"
BAR = "bar"
```

### init
initセクションは省略できます。

* addressとport
    * 接続するサーバーのアドレスとポートを指定します。デフォルトは127.0.0.1, 7530で、省略できます。
    * また、コマンドライン引数でも設定でき、その場合そちらが優先されます
* name
    * webcfaceでのこのlauncherの名前です。デフォルトは`webcface-launcher`で、省略できます。
    * コマンドライン引数でも設定できます。

### command
* name (必須)
    * webcfaceに表示される名前です
* exec (必須)
    * 実行するコマンドです
    * スペース区切りで引数も記述できます。
* workdir
    * コマンドを実行するディレクトリです。
    * 省略時カレントディレクトリになります
* stdout_capture <del>(tools ver1.3.1 から)</del>
    * <del>`"never"`, `"onerror"`(デフォルト), `"always"` が指定可能です</del>
    * <del>alwaysではコマンド終了時、onerrorではエラーで終了時にコマンドの標準出力とエラー出力の内容をlauncherのボタンの下に表示します</del>
    * (ver2.1 から) `true` の場合(デフォルト)、コマンドの標準出力とエラー出力の内容をLogデータとしてWebCFaceに送信します。
        * WebUIではメニューからコマンド名の名前のLog画面を開くことができます。
    * `false`にすると出力をキャプチャーしません。
    とくにwindowsではfalseにしないとうまく動作しないプログラムもあるようです。
* stdout_utf8 (windowsのみ、tools ver1.3.1 から)
    * falseの場合(デフォルト)、stdout_captureで取得したデータはANSIエンコーディングとみなし、UTF-8に変換してからWebCFaceに送られます。
    * trueの場合、stdout_captureで取得したデータをUTF-8とみなし、そのままWebCFaceに送ります。
* env (tools ver1.4 から)
    * 環境変数を設定します。
* stop (tools ver1.4.5 から)
    * Stopボタンを押したときの挙動を設定できます。
    * `stop = true` または `stop = 2` がデフォルトの挙動です。
        * Linux,MacOSではSIGINT(Ctrl+C)が送られます。
        * WindowsではTerminateProcessでプロセスツリー全体を停止します。
    * `stop = false` にするとStopボタンが無効になります。
    * (Linux,MacOSのみ) `stop = 9` などとすると送信するシグナルを変更できます。
        * Windowsでは指定したシグナルの番号にかかわらずTerminateProcessで停止します。
    * `stop.exec`を設定するとシグナルを送る代わりに指定したコマンドを実行します。
        * Startで実行するコマンドと同様、workdirやenvなどを設定することもできます。
        * 例えばlauncherから `cmd /c` などを使って別のプロセスを起動した場合、停止ボタンを押してcmdは停止してもその内側で起動したプロセスは停止しない場合があります。その場合はtaskkillなどで停止するコマンドを別途登録しておくとよいかも

## WebUIからの設定
WebCFace Desktop ではGUIから設定ファイルを記述することができます。

* Add New Command でコマンドを追加し、コマンド名、Exec、Workdir を設定できます。
    * Exec と Workdir の右の「...」ボタンを押すとファイル選択ダイアログ、ディレクトリ選択ダイアログが表示されます。
* Save ボタンを押すと WebCFace Desktop の設定として保存されます。
* Cancel ボタンを押すと最後にSaveしたときの内容に戻ります。

![launcher-setting](https://github.com/na-trium-144/webcface/raw/main/docs/images/launcher-setting.png)

\note Launcherの起動・停止はメニューの「Server Status」からできます

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [6-4. RobotModel](64_robot_model.md) | [7-2. webcface-send](72_send.md) |

</div>
