# 7-2. webcface-send

\tableofcontents
\since tools ver1.1

標準入力の内容をWebCFaceに送信します。
他のコマンドの出力をpipeで受け取りWebCFaceに流すことができます。

## コマンドライン引数
```
Usage: webcface-send [OPTIONS] [field]
```

* field: WebCFaceで表示するフィールド名(データの名前)です。省略時「data」になります
* `-h`: ヘルプを表示します。
* `-a address`: 接続するサーバーのアドレスです。省略時は127.0.0.1になります。
* `-p port`: 接続するサーバーのポートです。省略時は7530になります。
* `-m name`: WebCFaceでのメンバー名(WebUIで表示される名前)です。省略時は webcface-send になります。
* `-e`: (tools ver1.1.4 から) データをWebCFaceに送ると同時に標準出力にも流します。
* `-t type`: データの種類を指定します。`value`, `text`, `log`のいずれかが指定できます。省略時は`value`になります。
* `-8`: (windowsのみ、tools ver1.3.1 から) 入力データをutf-8とみなし、そのまま送ります。
    * 指定しない場合ANSIエンコーディングとみなし変換してから送られます。

## value

```sh
webcface-send [-t value]
```
標準入力に数値(半角の10進数)が送られるとそれをValueとして送信します。
改行で区切って繰り返し値を送れば順に送信されます。

## text

```sh
webcface-send -t text
```
標準入力の文字列を改行ごとにTextとして送信します。

## log

```sh
webcface-send -t log
```
標準入力の文字列を改行ごとにLogとして送信します。
ログレベルはすべて2(Info)になります。


