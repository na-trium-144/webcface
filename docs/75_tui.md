# 7-5. webcface-tui

\tableofcontents
\since tools ver2.0

WebUIの代わりとして、
ターミナルからValue, Textのデータをリアルタイムで確認したり、Viewの操作もできます。

![tui_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tui_value.gif)

![tui_view](https://github.com/na-trium-144/webcface/raw/main/docs/images/tui_view.png)

## コマンドライン引数
```
Usage: webcface-tui [OPTIONS] [field...]
```
* field: 表示するフィールド名(データの名前)です。
    * `メンバー名:データ名` の形で指定します。
    * 複数指定することもできます。
* `-h`: ヘルプを表示します。
* `-a address`: 接続するサーバーのアドレスです。省略時は127.0.0.1になります。
* `-p port`: 接続するサーバーのポートです。省略時は7530になります。
* `-w`: ライトテーマのターミナルを使っている場合これを指定することで指定すると白い背景に黒い文字が使われるようになります。
    * デフォルトでは黒い背景に白い文字であり、WebCFaceで指定した色とは反転した状態になります。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [7-4. webcface-ls](74_ls.md) | [7-6. webcface-notepad](76_notepad.md) |

</div>
