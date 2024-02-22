# webcface-cv-capture

\tableofcontents
\since tools ver1.3

Webカメラなどの画像をキャプチャーしてWebCFaceに送信します。

## コマンドライン引数
```
Usage: webcface-cv-capture [OPTIONS] [index] [field]
```
* index: `cv::VideoCapture()` の引数に渡す、デバイスのindexです。省略時0になります
 field: WebCFaceで表示するフィールド名(データの名前)です。省略時「data」になります
* `-h`: ヘルプを表示します。
* `-a address`: 接続するサーバーのアドレスです。省略時は127.0.0.1になります。
* `-p port`: 接続するサーバーのポートです。省略時は7530になります。
* `-m name`: WebCFaceでのメンバー名(WebUIで表示される名前)です。省略時は webcface-send になります。
* `-s width height`: キャプチャする画像のサイズです。
* `-r rate`: 画像のフレームレートです。
* `-f fourcc`: 画像のフォーマットです。
    * -s, -r, -f オプションの値はそのままOpenCVのVideoCapturePropertiesに渡されます。省略時はプロパティをセットしません。
