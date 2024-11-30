<sub><img src="https://raw.githubusercontent.com/na-trium-144/webcface-webui/main/public/icon.svg" height="40" /></sub>
WebCFace
====
<!-- ↑ Doxygenで見出しが崩れないように、かつGitHubではちゃんと見出しになるようにでっちあげたやつ -->

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=C%2B%2B)](https://github.com/na-trium-144/webcface)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)
[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)  
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/webcface?logo=Python&logoColor=white)](https://github.com/na-trium-144/webcface-python)
[![PyPI - Version](https://img.shields.io/pypi/v/webcface)](https://pypi.org/project/webcface/)  
[![javascript](https://img.shields.io/badge/JavaScript%2C%20TypeScript-gray?logo=JavaScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
[![npm](https://img.shields.io/npm/v/webcface)](https://www.npmjs.com/package/webcface)

Web-based IPC &amp; Dashboard-like UI

ROS1のようなプロセス間通信と、GUIによるデータの可視化や関数呼び出しができるシステムです。

C++ (C++17以上), C, Python (3.6以上), JavaScript/TypeScript で相互に数値、文字列、画像などのデータを送受信したり、関数を呼び出したりすることができます。

Linux, MacOS, Windows(MSVC, MinGW, MSYS2, Cygwin) で動作します。

## Features

### Easy to Setup

WebCFaceはサーバー側のプログラム `webcface-server` と、
クライアントライブラリで構成されています。  
使い方はサーバーを起動し、クライアントライブラリを利用したプログラムを起動するだけです。
ROSのようなワークスペース管理機能はなく、どんなフレームワークのプロジェクトにも簡単に組み込むことができます。

C / C++ の場合、WebCFaceのライブラリは
CMake を使っていれば `find_package(webcface)`、
pkg-config なら`pkg-config --cflags --libs webcface`
で簡単に利用できます。  
またライブラリ本体は
* Linux: `libwebcface.so.<version>`
* Mac: `libwebcface.<version>.dylib`
* Windows: `webcface-<version>.dll` (Release) or `webcfaced-<version>.dll` (Debug)

の1つのみであり、手動でこのライブラリにリンクして使うこともできます。  
WebCFace内部では外部ライブラリを多数使用していますが、それらはシンボルをすべて非公開にしているのでユーザーが使用するライブラリとは干渉しません。
(WebCFaceをソースからビルドした場合と、brewでインストールした場合を除く)

Python, JavaScript には PyPI / npm に `webcface` パッケージを用意しているのでそれをインストールするだけで使えます。
通信にWebSocketを使用しているため、Webブラウザ上でもそのまま動作します。

少し難易度は上がりますが、CのAPIを経由することで他の言語からも使用できると思います。

### Inter-Process Communication

WebCFaceの通信にはWebSocketとMessagePackを使っています。
このためプロセス間だけでなくWebブラウザーとの通信が可能になっています。
さらに同一マシン上やDocker,WSL経由など使用可能な場合はTCPの代わりにUnixドメインソケットを使用します。

Wi-FiやEtherNet経由で複数のPC間(OS問わず)で通信することも可能です。
WindowsとWSL1/2の間の相互通信も自動的に接続されます。

WebCFaceで送受信できるデータ型として
* 数値型・数値配列型(Value)
* 文字列型(Text)
* 画像(Image)
* 関数呼び出し(Func)
* テキストログ(Log)

などの型が用意されています。
ユーザーがメッセージ型を定義できるROSやgRPCと比べると自由度は低いかもしれませんが、
これらのデータ型の組み合わせであれば簡単に送受信させることができます。
コード例についてはドキュメントの
[1-2. Tutorial (Communication)](https://na-trium-144.github.io/webcface/md_docs_212__tutorial__comm.html)
を参照してください。

Image型データは送受信の過程で画像を縮小したりJPEGやPNGに圧縮したりといった操作をサーバー側で行うことができます。
表示目的など、圧縮した画像で十分な場合には簡単に通信量を削減できます。

WebCFaceの通信データ形式はOSやライブラリの言語によらず共通で、またバージョン間で後方互換性があります。
つまり、異なるバージョンのクライアント同士でも、異なるバージョンのOSでも問題なく通信が可能です。  
ただしそれぞれのクライアントのバージョンよりサーバーの方が新しいバージョンである必要があります。
サーバーよりクライアントのほうが新しい場合の動作は保証しません。
詳細は
[8-4. Versioning](https://na-trium-144.github.io/webcface/md_docs_284__versioning.html)
を参照してください。

### WebUI

WebCFaceではプログラム間でデータの送受信ができるAPIだけでなく、
WebブラウザーからWebCFaceで通信されているデータを可視化したり関数を呼び出したりできるUI(WebUI)を提供します。

さらにボタンや入力欄などの並べ方をWebCFaceを使ったC++,Pythonなどのプログラムの側で定義してそれをWebUIに表示させることができ、
これによりHTMLやCSSの知識がなくても簡易なUIを作成することができます。
コード例はドキュメントの
[1-1. Tutorial (Visualizing)](https://na-trium-144.github.io/webcface/md_docs_211__tutorial__vis.html)
を参照してください。

また、同様に2D、3Dの図形もWebCFaceを使ったプログラム側の記述のみでWebUIに描画させることができます。

![webcface-webui](https://raw.githubusercontent.com/na-trium-144/webcface/main/docs/images/webcface-webui.png)

これらの描画データは View, Canvas2D, Canvas3D として他のデータ型(数値や文字列など)と同様にWebCFace内の通信データとして存在しており、
WebUI以外でもこれらのデータを受信して表示するアプリを作成することは可能です。

### WebCFace-Tools

コマンドラインからWebCFaceのデータにアクセスできるコマンドもいくつか用意しています。

[webcface-launcher](https://na-trium-144.github.io/webcface/md_docs_271__launcher.html)
は事前に登録しておいたコマンドの起動・停止をWebCFaceのViewから操作することができる機能です。
serverとlauncherだけを常時起動しておき、WebUIを使ってプログラムをリモートに操作するという使い方ができます。

![launcher.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/launcher.png)

[webcface-tui](https://na-trium-144.github.io/webcface/md_docs_275__tui.html)
はターミナル上で操作できるTUIアプリで、Webブラウザを開かなくてもデータをリアルタイムで確認したり、Viewの操作もできます。

![tui_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tui_value.gif)

![tui_view](https://github.com/na-trium-144/webcface/raw/main/docs/images/tui_view.png)

<!--
### PlotJuggler

[plotjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin) を使うと、WebCFaceで通信されているデータを [PlotJuggler](https://github.com/facontidavide/PlotJuggler) を使って見ることもできます。
-->

<!--
todo: グラフなど使ってわかりやすくする & ほかの通信ライブラリとの比較をする

### Benchmark

ver1.11時点のReleaseビルドの src/example/benchmark.cc で通信速度をチェックしてみました。
以下の表は クライアント→サーバー→クライアント でさまざまなサイズの文字列データの送受信にかかった時間です。
なおこれはサーバーとクライアントが同一マシン上の場合の結果なので、Wi-FiやEthernetを経由する場合はその環境次第ですがこれより遅くなると思います。

使用したPCのCPUは、MacOSは Apple M1 、それ以外は Intel Core i5-13500 です。

<details><summary>表</summary>

| OS | 10Byte | 100Byte | 1kByte | 10kByte | 100kByte | 1MByte |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| Windows (MSVC build) | 244 μs | 301 μs | 381 μs | 428 μs | 2.82 ms | 22.5 ms |
| Windows (MinGW build) | 219 μs | 218 μs | 262 μs | 411 μs | 1.93 ms | 17.0 ms |
| Linux (on WSL1) | 177 μs | 213 μs | 195 μs | 272 μs | 1.26 ms | 12.3 ms |
| Server=MSVC + Client=WSL1 | 323 μs | 258 μs | 401 μs | 420 μs | 2.34 ms | 18.2 ms |
| Server=MSVC + Client=WSL2 | 379 μs | 369 μs | 488 μs | 656 μs | 2.47 ms | 17.6 ms |
| Server=WSL1 + Client=MSVC | 335 μs | 287 μs | 252 μs | 504 μs | 2.02 ms | 16.3 ms |
| Server=WSL2 + Client=MSVC | 553 μs | 637 μs | 622 μs | 810 μs | 2.28 ms | 29.5 ms |
| Linux (Native) | 491 μs | 439 μs | 519 μs | 1.01 ms | 4.78 ms | 27.8 ms |
| MacOS | 130 μs | 136 μs | 165 μs | 439 μs | 2.98 ms | 28.3 ms |

</details>
-->

## Documentation

DoxygenでAPIリファレンスとともにチュートリアル、ドキュメントを公開しています。

* ダウンロード、インストール方法はREADME(この下)にあります
* 1 チュートリアル: [Visualizing](https://na-trium-144.github.io/webcface/md_docs_211__tutorial__vis.html) / [Communication](https://na-trium-144.github.io/webcface/md_docs_212__tutorial__comm.html)
* 2 [Server](https://na-trium-144.github.io/webcface/md_docs_221__server.html)
/ [WebUI](https://na-trium-144.github.io/webcface/md_docs_222__webui.html)
* 3 [クライアントライブラリのセットアップ](https://na-trium-144.github.io/webcface/md_docs_231__setup.html)
/ [WebCFaceのソースからのビルド](https://na-trium-144.github.io/webcface/md_docs_232__building.html)
* [C/C++ APIリファレンス](https://na-trium-144.github.io/webcface/namespaces.html)
* [webcface-python APIリファレンス](https://na-trium-144.github.io/webcface-python/)
* [webcface-js APIリファレンス](https://na-trium-144.github.io/webcface-js/)

## Links

* [webcface](https://github.com/na-trium-144/webcface): サーバー & C/C++クライアントライブラリ (このリポジトリ)
* [webcface-webui](https://github.com/na-trium-144/webcface-webui): webブラウザ用UIアプリ
* [webcface-tools](https://github.com/na-trium-144/webcface-tools): コマンドラインツール群
* [webcface-js](https://github.com/na-trium-144/webcface-js): JavaScriptクライアントライブラリ
* [webcface-python](https://github.com/na-trium-144/webcface-python): Pythonクライアントライブラリ
* [webcface-package](https://github.com/na-trium-144/webcface-package): ビルドしたサーバー、ライブラリ、WebUI、Tools をアーカイブ化してリリースしている場所
* [homebrew-webcface](https://github.com/na-trium-144/homebrew-webcface): HomebrewのTapを管理しています
<!-- * [plogjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin): PlotJuggler プラグイン -->

## Installation

WebCFace ver1については [v1ブランチ](https://github.com/na-trium-144/webcface/tree/v1?tab=readme-ov-file#installation) を参照してください。

ver2は以下のようにLinux,Windows,MacOS用にビルドしたアーカイブをダウンロードできます。

### Ubuntu (x86_64, arm64, armhf)

[webcface-package リポジトリから](https://github.com/na-trium-144/webcface-package/releases)
linux用のzipファイルをダウンロードし、任意の場所に展開してください。
* sudo権限が使える場合は /usr/local または /opt/webcface に展開するのがおすすめです。
* また、展開したディレクトリ内の bin/ をPATHに、 lib/\*-linux-gnu\*/pkgconfig/ をPKG_CONFIG_PATHに追加してください。
* さらにsystemdのサービスファイルを使用したい場合は /opt/webcface/lib/systemd/system に展開されたファイルに対して /etc/systemd/system/ にリンクを貼るなどしてください。

Ubuntu20.04でビルドしているため、それより古いUbuntuでは動作しません(ソースからビルドする必要があります)。
また、Ubuntu以外のディストリビューションで動作するかはわかりません。

ダウンロードと展開をコマンドラインでやるなら以下のようにします。
(/opt/webcface に展開し, 環境変数を ~/.bashrc に書き込みます。それ以外の環境の場合は適宜読み替えてください。)

<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.5.0-2/webcface_2.5.0-2_linux_amd64.zip
sudo unzip webcface_2.5.0-2_linux_amd64.zip -d /opt/webcface
rm webcface_2.5.0-2_linux_amd64.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.bashrc
sudo ln -sf /opt/webcface/lib/systemd/system/*.service /etc/systemd/system/
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.5.0-2/webcface_2.5.0-2_linux_arm64.zip
sudo unzip webcface_2.5.0-2_linux_arm64.zip -d /opt/webcface
rm webcface_2.5.0-2_linux_arm64.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.bashrc
sudo ln -sf /opt/webcface/lib/systemd/system/*.service /etc/systemd/system/
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.5.0-2/webcface_2.5.0-2_linux_armhf.zip
sudo unzip webcface_2.5.0-2_linux_armhf.zip -d /opt/webcface
rm webcface_2.5.0-2_linux_armhf.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/arm-linux-gnueabihf/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.bashrc
sudo ln -sf /opt/webcface/lib/systemd/system/*.service /etc/systemd/system/
```
</details>


### Deb Package (Ubuntu x86_64, arm64, armhf)

Debパッケージとしてビルドしたものを [WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からダウンロードしてインストールできます。
内容はzipアーカイブで配布しているものに加えて WebCFace Desktop のアプリケーションランチャーが含まれます。

Ubuntu20.04でビルドしているため、それより古いUbuntuでは動作しません(ソースからビルドする必要があります)。
また、Ubuntu以外のディストリビューションで動作するかはわかりません。

ダウンロードと展開をコマンドラインでやるなら以下のようにします。

<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v2.5.0/webcface_2.5.0_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v2.1.2/webcface-tools_2.1.2_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.10.2/webcface-webui_1.10.2_all.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.10.2/webcface-desktop_1.10.2_linux_amd64.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v2.5.0/webcface_2.5.0_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v2.1.2/webcface-tools_2.1.2_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.10.2/webcface-webui_1.10.2_all.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.10.2/webcface-desktop_1.10.2_linux_arm64.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v2.5.0/webcface_2.5.0_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v2.1.2/webcface-tools_2.1.2_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.10.2/webcface-webui_1.10.2_all.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.10.2/webcface-desktop_1.10.2_linux_armv7l.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
</details>

### macOS

[webcface-package リポジトリから](https://github.com/na-trium-144/webcface-package/releases)
macos用のzipファイルをダウンロードできますが、
署名や公証をしていないためブラウザーからダウンロードするとGatekeeperにブロックされてしまいます。
(開発元を検証できないため開けません。の画面になります)

以下のようにコマンドラインからダウンロード、展開することでGatekeeperを回避できます。

Universalバイナリになっており、IntelMacもAppleシリコンも共通です。
macOS 12 (Monterey) でビルドしているので、それより古いMacでは動かないかもしれません。

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.5.0-2/webcface_2.5.0-2_macos_universal.zip
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.5.0-2/webcface-desktop_2.5.0-2_macos_app.zip
```

sudo権限が使用できれば以下のように webcface_universal を /opt/webcface に、 webcface-desktop_app を /Applications に展開するのがおすすめです。
```sh
sudo unzip webcface_2.5.0-2_macos_universal.zip -d /opt/webcface
sudo unzip webcface-desktop_2.5.0-2_macos_app.zip -d /Applications
rm webcface_2.5.0-2_macos_universal.zip
rm webcface-desktop_2.5.0-2_macos_app.zip
```

また、展開したディレクトリ内の bin/ をPATHに、 lib/pkgconfig/ をPKG_CONFIG_PATHに追加してください。
(別の場所に展開した場合や、zsh以外の環境の場合は適宜読み替えてください)
```sh
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.zshrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.zshrc
```

webcfaceライブラリはinstall_nameが /opt/webcface/lib のパスになっているため、それ以外の場所に展開した場合は
`export DYLD_LIBRARY_PATH="(webcfaceディレクトリのパス)/lib:$DYLD_LIBRARY_PATH"`
も必要になるかもしれません。

### Homebrew (MacOS, Linux)

[na-trium-144/webcface のtap](https://github.com/na-trium-144/homebrew-webcface) からインストールできますが、
brewでビルドしたwebcfaceはsharedライブラリとして多数の依存ライブラリが必要になるのであまりおすすめしません。

また、brewでインストールした場合 WebCFace Desktop アプリは付属しません。
```sh
brew tap na-trium-144/webcface
brew install webcface webcface-webui webcface-tools
```

### Windows MSVC

[webcface-package リポジトリから](https://github.com/na-trium-144/webcface-package/releases)
windows用のexeファイルまたはzipファイルをダウンロードできます。
x86バージョンとx64バージョンがあります。
(ただしインストーラーと WebCFace Desktop アプリはどちらも32bitになっています)
* exeファイルは実行するとインストーラーが起動します。
    * 署名していないため Windows Defender にブロックされるかもしれません。
    その場合は「詳細情報」→「実行」をクリックして実行してください。
    * インストール場所はデフォルトで C:\Program Files\webcface になります。(変更可能です)
    * また、自動的に環境変数のPATHが設定され、スタートメニューにも WebCFace Desktop のショートカットが追加されます。
    * アンインストールはコントロールパネルや設定アプリから他のアプリと同様にできます。
* zipファイルは任意の場所に展開して使用してください。
    * コマンドラインツールやライブラリを使う(Meson,CMakeでインポートする)には、展開したwebcfaceディレクトリの中の bin/ を手動で環境変数のPATHに追加する必要があります。
* [最新の Visual C++ 再頒布可能パッケージ](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-microsoft-visual-c-redistributable-version)
がインストールされていない場合はインストールする必要があります。
* 比較的新しいWindows10以上であれば動作するはずです。古いWindowsでは動作確認していません。
* いずれも最新バージョンの Visual Studio 2019 でビルドしているため、それよりも古い Visual Studio からwebcfaceライブラリにリンクすると正常動作しないかもしれません。

MinGW用バイナリは今のところ配布していません(ソースからビルドしてください)

## License

WebCFaceと関連するプログラムはすべてMITライセンスで公開しています。詳細は [LICENSE](https://github.com/na-trium-144/webcface/blob/main/LICENSE) を参照してください。

WebCFace本体とtoolsが使用しているサードパーティーのライブラリのライセンスはそれぞれ以下を参照してください。
* msgpack-c (Boost Software License) : https://github.com/msgpack/msgpack-c
* eventpp (Apache 2.0) : https://github.com/wqking/eventpp
* spdlog (MIT) : https://github.com/gabime/spdlog
* curl : https://curl.se/docs/copyright.html
* Asio (Boost Software License) : http://think-async.com/Asio/
* Crow (BSD 3-Clause) : https://github.com/CrowCpp/Crow
* CLI11 (BSD 3-Clause) : https://github.com/CLIUtils/CLI11
* UTF8-CPP (BSD 1.0) : https://github.com/nemtrif/utfcpp
* ImageMagick: https://imagemagick.org/script/license.php
* tiny-process-library (MIT) : https://gitlab.com/eidheim/tiny-process-library
* toml++ (MIT) : https://github.com/marzer/tomlplusplus
* FTXUI (MIT) : https://github.com/ArthurSonzogni/FTXUI
