# 8-4. Versioning

\tableofcontents

## Message Compatibility

C/C++のWebCFaceライブラリと、JavaScript、PythonのWebCFaceライブラリはバージョン番号が異なりますが、
通信されるメッセージ仕様の互換性は以下のとおりです。

それぞれのクライアントのバージョンと比べてサーバーのバージョンが同じか新しい必要があります。
サーバーよりクライアントのほうが新しい場合の動作は保証しません。

| Server, C/C++ | Python | JavaScript | WebUI | 備考 |
| ---- | ---- | ---- | ---- | ---- |
| 1.0 | | | | |
| 1.1〜1.2 | 1.0 | 1.0〜1.1 | 1.0 | |
| 1.3 | | 1.2 | 1.1 | Image 追加 |
| 1.4〜1.5 | | 1.3 | 1.2〜1.3 | RobotModel, Canvas3D 追加 |
| 1.6〜1.8 | 1.1 | 1.4 | 1.4 | Canvas2D 追加 |
| 1.9 | | 1.5 | 1.5 | Canvas2DのonClickプロパティ |
| 1.10〜1.11 | | 1.6 | 1.6〜1.7 | Viewのinput要素 |
| 2.0 | | 1.7 | 1.8.0 | SyncInitEnd |
| 2.1〜2.3 | 2.0 | 1.8 | 1.8.1〜1.8.3 | LogEntry |
| 2.4 | 2.1〜2.4 | 1.9 | 1.9〜1.10 | Log (kind=85 → 8) |
| 2.5 | 3.0 | | | View (3→9), Canvas2D (4→10), 3D (7→11) |

## ABI Compatibility

WebCFaceは共有ライブラリであり、WebCFaceを使ったアプリケーションをビルドしたときと異なるバージョンのWebCFaceライブラリを使って実行することもできます。
ABIのメジャーバージョン(小数点より左)が一致し、かつマイナーバージョン(小数点より右)がビルド時より新しいものであれば正常動作することが保証されます。

ABIのメジャーバージョンはライブラリのファイル名に
* Linux: `libwebcface.so.<version>`
* Mac: `libwebcface.<version>.dylib`
* Windows: `webcface-<version>.dll` (Release) or `webcfaced-<version>.dll` (Debug)

のように含まれています。

ABIのマイナーバージョンはMacで `otool` などを使った場合にのみ確認することができます。

| API version | ABI version | ビルド済みの webcface-tools バイナリの Release |
| ---- | ---- | ---- |
| 1.0〜1.1 | 1 | 1.0〜1.1.4 |
| 1.2 | 2 | 1.1.5〜1.1.7 |
| 1.3〜1.4 | 3 | 1.1.7〜1.2 |
| 1.5 | 5 | 1.3.0〜1.3.1 |
| 1.6 | 6 | 1.3.1〜1.4.2 |
| 1.7 | 7 | |
| 1.8 | 8 | 1.4.2-1 |
| 1.9 | 9 | 1.4.2-2 |
| 1.10 | 10 | 1.4.3 |
| 1.11 | 11 | 1.4.4 |
| 2.0.0〜2.0.1 | 20.0 | 2.0 |
| 2.0.2〜2.0.5 | 20.1 | |
| 2.1.0〜2.3.0 | 20.2 | |
| 2.4.0〜2.4.1 | 20.3 | 2.1.0〜2.1.1 |
| 2.4.2 | 20.4 | |
| 2.5 | 21.0 | 2.1.2〜 |

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [8-3. Server Spec](83_server_spec.md) | |

</div>