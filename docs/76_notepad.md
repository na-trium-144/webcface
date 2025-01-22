# 7-6. webcface-notepad

\tableofcontents
\since tools ver2.2

Viewのテキスト入力機能を使ったメモ帳です。
WebUIや webcface-tui で入力したテキストをファイルに保存できます。

![notepad](https://github.com/na-trium-144/webcface/raw/main/docs/images/notepad.png)

## コマンドライン引数
```
Usage: webcface-notepad [OPTIONS] [filename]
```
* `-h`: ヘルプを表示します。
* `-a address`: 接続するサーバーのアドレスです。省略時は127.0.0.1になります。
* `-p port`: 接続するサーバーのポートです。省略時は7530になります。
* `-s width height`: テキスト入力欄のサイズを変更できます。省略時は 30 10 です。
* `filename`: 入力されたテキストをファイルに保存します。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [7-5. webcface-tui](75_tui.md) | [8-1. Message](81_message.md) |

</div>
