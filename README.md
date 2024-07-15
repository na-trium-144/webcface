# WebCFace

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=C%2B%2B)](https://github.com/na-trium-144/webcface)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)
[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)  
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/webcface?logo=Python&logoColor=white)](https://github.com/na-trium-144/webcface-python)
[![PyPI - Version](https://img.shields.io/pypi/v/webcface)](https://pypi.org/project/webcface/)  
[![javascript](https://img.shields.io/badge/JavaScript%2C%20TypeScript-gray?logo=JavaScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
[![npm](https://img.shields.io/npm/v/webcface)](https://www.npmjs.com/package/webcface)

Web-based IPC &amp; Dashboard-like UI

> * mainブランチはver2.0.0としてリリース予定の現在開発中のブランチです。
ver1は [v1](https://github.com/na-trium-144/webcface/tree/v1) ブランチにあります

WebSocketとMessagePackを使った、ROSのような分散型の通信ライブラリです。

C++ (C++17以上), C (C99), Python (3.8以上), JavaScript/TypeScript で相互に数値、文字列、画像などのデータを送受信したり、関数(手続き)を呼び出したりすることができます。

Linux, MacOS, Windows(MSVC, MinGW, MSYS2, Cygwin) で動作します。
Wi-FiやEtherNet経由で複数のPC間(OS問わず)で通信することも可能です。
WindowsとWSL1/2の間の相互通信も自動的に接続されます。

さらに同一マシン上やDocker,WSL経由など使用可能な場合はTCPの代わりにUnixドメインソケットを使用するようにし、パフォーマンスが改善しました。

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
* Mac: `webcface.framework` (または `libwebcface.<version>.dylib`)
* Windows: `webcface<version>.dll` (MinGWの場合 `libwebcface<version>.dll`)

の1つのみであり、手動でこのライブラリにリンクして使うこともできます。  
WebCFace内部では外部ライブラリを多数使用していますが、それらはシンボルをすべて非公開にしているのでユーザーが使用するライブラリとは干渉しません。
(WebCFaceをソースからビルドした場合と、brewでインストールした場合を除く)

Python, JavaScript には PyPI / npm に `webcface` パッケージを用意しているのでそれをインストールするだけで使えます。
通信にWebSocketを使用しているため、Webブラウザ上でもそのまま動作します。

少し難易度は上がりますが、CのAPIを経由することで他の言語からも使用できると思います。

### Inter-Process Communication

WebCFaceで送受信できるデータ型として
* 数値型・数値配列型(Value)
* 文字列型(Text)
* 画像(Image)
* 関数呼び出し(Func)
* テキストログ(Log)

などの型が用意されています。
ユーザーがメッセージ型を定義できるROSやgRPCと比べると自由度は低いかもしれませんが、
これらのデータ型の組み合わせであれば簡単に送受信させることができます。

Image型データは送受信の過程で画像を縮小したりJPEGやPNGに圧縮したりといった操作をサーバー側で行うことができます。
表示目的など、圧縮した画像で十分な場合には簡単に通信量を削減できます。

WebCFaceの通信データ形式はOSやライブラリの言語によらず共通で、またバージョン間で後方互換性があります。
つまり、異なるバージョンのクライアント同士でも、異なるバージョンのOSでも問題なく通信が可能です。
(C++のver1.1.6以前を除く)  
ただしそれぞれのクライアントのバージョンよりサーバーの方が新しいバージョンである必要があります。

### WebUI

WebCFaceではプログラム間でデータの送受信ができるAPIだけでなく、
WebブラウザーからWebCFaceで通信されているデータを可視化したり関数を呼び出したりできるUI(WebUI)を提供します。

さらにボタンや入力欄などの並べ方をWebCFaceを使ったC++などのプログラムの側で定義してそれをWebUIに表示させることができ、
これによりHTMLやCSSの知識がなくても簡易なUIを作成することができます。

また、同様に2D、3Dの図形もWebCFaceを使ったプログラム側の記述のみでWebUIに描画させることができます。

![webcface-webui](https://raw.githubusercontent.com/na-trium-144/webcface/main/docs/images/webcface-webui.png)

なお、これらの描画データは View, Canvas2D, Canvas3D として他のデータ型(数値や文字列など)と同様にWebCFace内の通信データとして存在しており、
WebUI以外でもこれらのデータを受信して表示するアプリを作成することは可能です。

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

## Links

* [webcface](https://github.com/na-trium-144/webcface): サーバー & C/C++クライアントライブラリ (このリポジトリ)
* [webcface-webui](https://github.com/na-trium-144/webcface-webui): webブラウザ用UIアプリ
* [webcface-tools](https://github.com/na-trium-144/webcface-tools): コマンドラインツール群
* [webcface-js](https://github.com/na-trium-144/webcface-js): JavaScriptクライアントライブラリ
* [webcface-python](https://github.com/na-trium-144/webcface-python): Pythonクライアントライブラリ
* [homebrew-webcface](https://github.com/na-trium-144/homebrew-webcface): HomebrewのTapを管理しています
* [webcface-windows-package](https://github.com/na-trium-144/webcface-windows-package): Windows用ビルドをリリースする場所
* [plogjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin): PlotJuggler プラグイン

## Installation
以下の手順で webcface, webcface-webui, webcface-tools をインストールできます。
(webcface-toolsの内容と使い方についてはwebcface-toolsのReadmeを参照してください)

### Ubuntu (x86_64, arm64, armhf)
[WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からそれぞれ最新のdebパッケージをダウンロードしてインストールできます。

debパッケージはubuntu20.04,22.04,24.04でビルドしています。
それぞれ依存するパッケージのバージョンが違います。
Debianなど他のディストリビューションで動作するかはわかりません。

コマンドからダウンロードするなら以下のようにします。

#### Ubuntu 24.04 Noble
<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu24.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu24.04_amd64.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu24.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu24.04_arm64.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu24.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu24.04_armhf.deb
```
</details>

#### Ubuntu 22.04 Jammy
<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu22.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu22.04_amd64.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu22.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu22.04_arm64.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu22.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu22.04_armhf.deb
```
</details>

#### Ubuntu 20.04 Focal
<details><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu20.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu20.04_amd64.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu20.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu20.04_arm64.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu20.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.5/webcface-tools_1.4.5-ubuntu20.04_armhf.deb
```
</details>

ダウンロードできたら
```sh
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
でインストールできます

### Homebrew (MacOS, Linux)
```sh
brew tap na-trium-144/webcface
brew install webcface webcface-webui webcface-tools
```

### App Bundle for MacOS
[homebrew-webcfaceのRelease](https://github.com/na-trium-144/homebrew-webcface/releases) から .app.zip ファイルをダウンロードするか、
```sh
brew tap na-trium-144/webcface
brew install --cask webcface-webui-server
```
でインストールできます。

インストールしたappを起動するとWebUIのウィンドウとwebcface-serverが起動します。(詳細はドキュメントの [Overview](https://na-trium-144.github.io/webcface/md_00__overview.html) を参照)


### Windows (MSVC)
[webcface-windows-packageのRelease](https://github.com/na-trium-144/webcface-windows-package/releases)からダウンロードしてください。

exeファイルは実行するとインストーラーが起動します。
zipファイルは任意の場所に展開して使用してください。

(ver2.0〜) Windows版WebCFaceのReleaseに含まれるビルド済みのspdlogは
`SPDLOG_WCHAR_SUPPORT` オプションがオンの状態です

MinGW用バイナリは今のところ配布していません(ソースからビルドしてください)

## Build from source

以下はwebcfaceをソースからビルドする場合の説明です。(webcfaceをインストールした場合は不要です。)

### Requirements
* C++17に対応したコンパイラが必要です
	* (ver1.11まではC++20が必要でしたが、ver2からC++17に移行しました)
* Linuxはgcc-7以上とclang-7以上、MacはmacOS12(Monterey)以上、Visual Studioは2019以上でビルドできることを確認しています。
それ以前のバージョンでも動くかもしれません。
* MinGWでもビルドできます。MSYS2のMINGW64環境でテストしていますがUCRT64やCLANG64環境でもビルドできると思います。
* Cygwin(64bit)やMSYS2のMSYS環境ではasioが非対応なため、現状ではサーバー機能を除いてクライアントのみビルドすることができます。CMake時に`-DWEBCFACE_SERVER=OFF`が必要です。
([chriskohlhoff/asio#518](https://github.com/chriskohlhoff/asio/issues/518))

### Dependencies
* webcfaceは外部ライブラリとして
[msgpack-cxx](https://github.com/msgpack/msgpack-c),
[eventpp](https://github.com/wqking/eventpp),
[spdlog](https://github.com/gabime/spdlog),
[curl](https://github.com/curl/curl),
[crow](https://github.com/CrowCpp/Crow),
[asio](https://github.com/chriskohlhoff/asio),
[cli11](https://github.com/CLIUtils/CLI11.git),
[UTF8-CPP](https://github.com/nemtrif/utfcpp),
[OpenCV](https://opencv.org/)(exampleのみ),
[Magick++](https://github.com/ImageMagick/ImageMagick),
[GoogleTest](https://github.com/google/googletest)(testのみ)
を使用します。
* いずれもシステムにインストールされていてfind_packageなどで使うことができるか確認します。
	* 特にsharedまたはstaticライブラリとしてビルドできるspdlog,libcurl,Magick++は事前にインストールしておいたほうがビルドが高速になります。
		* ただしインストールされたlibcurl,Magick++を使用するには条件があります。(後述)
	* 見つからない場合は、CMake時に自動的にFetchContentでソースコードを取得してビルドするので、必ずしもこれらをインストールする必要はありません。
* また、`git submodule update --init` をしておくとsubmoduleとしてexternalディレクトリ以下に依存ライブラリをすべてcloneすることができます。その場合はFetchContentでのダウンロードはされません。
	* ダウンロード量は増えますが、インターネット接続のないところでcmakeをしたい場合などに有用です。
	* ただしMSVCの場合ImageMagick-Windowsリポジトリは例外として常にFetchContentを使います
* システムにインストールされているものを使用した場合、ビルドしたWebCFaceはそのライブラリに依存することになります。
一方FetchContentまたはsubmoduleでソースからビルドし、かつWebCFaceがsharedライブラリとしてビルドされる場合は、外部ライブラリのシンボルがWebCFace内部に隠されます。
	* そのためビルドしたWebCFaceを他の環境に配布する場合などはシステムにインストールしたライブラリを使用しないほうが良いです。
		* CMake時に`-DWEBCFACE_FIND_LIBS=OFF`とすることですべてソースからビルドさせることができます。
* libcurlはwebsocket機能を有効にする必要があるため、インストールされているlibcurlでwebsocketが使えない場合使用せずソースからビルドします。
* crowはunix_socketの機能が実装されている必要があるため、インストールされているcrowでunix_socketが使えない場合使用せずソースからビルドします
* Magick++はマルチスレッドで実行するためにOpenMPが無効になっている必要があるため、インストールされているMagick++がOpenMPを使用してビルドされていた場合使用せずソースからビルドします
* OpenCVはソースからビルドしません。OpenCVを使ったexampleをビルドしたい場合は別途インストールする必要がありますが、example以外では使用しないのでほぼ必要ないと思います。

<details><summary>Ubuntu</summary>

```sh
sudo apt install build-essential git cmake pkg-config
# optional:
sudo apt install libspdlog-dev libasio-dev
sudo apt install libcli11-dev # only on 22.04 or later
sudo apt install libmsgpack-cxx-dev # only on 24.04 or later
```

</details>

<details><summary>Homebrew (MacOS, Linux)</summary>

```sh
brew install cmake nasm
# optional:
brew install msgpack-cxx spdlog asio cli11 utf8cpp
```
libjpegをソースからビルドする際にnasmがあるとよいです

</details>

<details><summary>Visual Studio</summary>

Visual Studio 2019 または 2022 でcloneしたwebcfaceのフォルダーを開くか、
Developer Command Promptからcmakeコマンドを使ってもビルドできます。

<!--
https://imagemagick.org/script/download.php からImageMagickをダウンロード、インストールしてPATHを通せばそれを使用してビルドすることができます。
(インストール時に development header もインストールすること)

または、chocolateyをインストールしてあれば `choco install imagemagick -PackageParameters InstallDevelopmentHeaders=true` でok
ImageMagickをインストールしない場合CMake時に自動的にソースをダウンロードしてビルドします。
-->
ImageMagickをソースからビルドするために Visual C++ ATL と MFC のコンポーネントも必要になります。
(Visual Studio Installer からインストールしてください)  
公式サイトからダウンロードできるImageMagickはOpenMPを使ってビルドされているため、インストールされていても使いません。
(またはImageMagickを別途 /noOpenMP オプション付きでビルドしたものを用意しPATHを通せばそれを使用することは可能です)

</details>

<details><summary>MSYS2 MinGW</summary>

```sh
pacman -S pactoys
pacboy -S git make gcc:p cmake:p ninja:p
# optional:
pacboy -S msgpack-cxx:p spdlog:p asio:p cli11:p utf8cpp:p
```
imagemagickをソースからビルドする際にninjaではなくmakeが必要になります

</details>

<details><summary>MSYS2 MSYS</summary>

```sh
pacman -S git make gcc cmake ninja
```
imagemagickをソースからビルドする際にninjaではなくmakeが必要になります

</details>

<details><summary>Cygwin</summary>

gcc-core, gcc-g++, cmake, make, pkg-config, (ninja) をインストールしてください
(ninjaでもいいですが、imagemagickをソースからビルドする際にはninjaではなくmakeが必要になります)

</details>

### CMake

```sh
cmake -Bbuild
```
* ImageMagickなど、CMakeを使わない依存ライブラリのビルドはCMake時に行われます。そのため初回のCMakeで大量のログ出力が流れ時間もかかりますが仕様です。
* CMakeの最後に `** WebCFace (バージョン番号) Summary**` と指定されているオプション、依存ライブラリの状態がまとめて表示されます。
* BUILD_TYPEは `-DCMAKE_BUILD_TYPE=Release` または `-DCMAKE_BUILD_TYPE=Debug` を指定してください (デフォルトはRelease)
	* Windows(MSVC)ではImageMagickをソースからビルドする際デフォルトでは`CMAKE_BUILD_TYPE`に指定したconfigurationのみビルドされますが、DebugとReleaseの両方をビルドしたい場合は `-DWEBCFACE_CONFIG_ALL=ON` を指定してください
* `-DWEBCFACE_SHARED=OFF`にすると共有ライブラリではなくすべて静的ライブラリになります
* `-DWEBCFACE_PIC=ON`または`-DCMAKE_POSITION_INDEPENDENT_CODE=ON`にすると-fPICフラグが有効になります (Linux,Macのみ、WEBCFACE_SHAREDがONの場合はデフォルトでON)
* `-DWEBCFACE_FRAMEWORK=ON`, `"-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"` などとするとユニバーサルなframework (webcface.framework) をビルド、インストールできます
* `-DWEBCFACE_SERVER=ON`でserverをビルドします(デフォルトでON)
* `-DWEBCFACE_EXAMPLE=ON`でexampleをビルドします(デフォルトでON、subdirectoryの場合デフォルトでOFF)
* `-DWEBCFACE_INSTALL=ON`でinstallターゲットを生成します(デフォルトでON、subdirectoryの場合デフォルトでOFF)
	* さらに`-DWEBCFACE_INSTALL_SERVICE=ON`で [webcface-server.service](cmake/webcafce-server.service) を lib/systemd/system にインストールします (デフォルトでOFF)
	* インストール先はデフォルトで /usr/local ですが、 `-DCMAKE_INSTALL_PREFIX=~/.webcface` などと指定すると変更することができます
* `-DWEBCFACE_TEST=ON`でtestをビルドします(デフォルトでOFF)
	* テストが通らない場合テスト中の通信の待機時間を`-DWEBCFACE_TEST_TIMEOUT=100`などと伸ばすとうまく行く場合があります(デフォルト=10(ms))
* `-DWEBCFACE_VERSION_SUFFIX`でバージョン表記を変更できます
	* 例えばバージョン(common/def.hに定義されている)が1.2.0のとき
	* `-DWEBCFACE_VERSION_SUFFIX=git` なら `git describe --tags` コマンドを使用して取得した文字列 (1.2.0-x-gxxxxxxx) になります(未指定の場合のデフォルト)
	* `git`以外の任意の文字列の場合 `-DWEBCFACE_VERSION_SUFFIX=hoge` で 1.2.0-hoge になります
	* `-DWEBCFACE_VERSION_SUFFIX=` で 1.2.0 だけになります
* `-DWEBCFACE_WARNING=ON`でコンパイラのWarningをすべてONにし、さらにほとんどのWarningをErrorにします(デフォルトでON、subdirectoryの場合デフォルトでOFF)
* `-DWEBCFACE_CLANG_TIDY=/path/to/clang-tidy`でコンパイル時にclang-tidyのチェックを実行します(空文字列で無効、デフォルトで無効)
	* チェックの設定は .clang-tidy ファイル参照
* `-DWEBCFACE_DOWNLOAD_WEBUI=ON`でCMake時にWebUIをダウンロードします。(デフォルトでON、subdirectoryの場合デフォルトでOFF)
* 依存ライブラリ
	* `-DWEBCFACE_FIND_(ライブラリ)=OFF` にするとfind_packageなどで依存ライブラリを確認せず、常にソースからビルドするようになります。
	* 設定可能なライブラリ名は以下
		* `MSGPACK`, `SPDLOG`, `EVENTPP`, `CURL`, `CROW`, `ASIO`, `CLI11`, `UTF8CPP`, `MAGICK`
		* `OPENCV` (デフォルトでoff、見つからなかった場合ソースからのビルドもしません)
		* Magickをソースビルドする場合のみ: `JPEG`, `PNG`, `ZLIB`, `WEBP`
	* `-DWEBCFACE_FIND_LIBS=OFF` とすると上記設定をすべてOFFにします
		* WEBCFACE_SHAREDがOFFの場合デフォルトでOFF

### Build

```sh
cmake --build build
```
でビルドします
(Windows(MSVC)の場合は `--config Release` または `--config Debug` を追加してください。)

インストールもしたい場合は
```sh
sudo cmake --build build -t install
```
でできます
(Windows(MSVC)の場合は `--config Release` または `--config Debug` を追加してください。)

### WebUI

デフォルトではビルド済みのものがCMake時にダウンロードされますが、
`-DWEBCFACE_DOWNLOAD_WEBUI=OFF` を指定した場合はダウンロードしません。
その場合は [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードしてください。

インストールする場合は (webcfaceのインストール場所)/share/webcface/dist として展開してください。
(install先が/usr/localでない場合はprefixを読み替えてください)

installせずに実行する場合は webcface-server のバイナリと同じディレクトリか、その1, 2, 3階層上のどこかにdistディレクトリを配置してください。

Ubuntuの場合は[webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases)にあるdebパッケージで、またhomebrewでは `webcface-webui` パッケージとしてWebUIだけを単体でインストールすることもできます。

または自分でビルドすることも可能です。(node.jsが必要)
[webcface-webui](https://github.com/na-trium-144/webcface-webui) のREADMEを参照してください。

### tools

toolsは別途 https://github.com/na-trium-144/webcface-tools.git をcloneしてビルド、インストールしてください

## Documentation
* まずはここから→ [Overview](https://na-trium-144.github.io/webcface/md_00__overview.html)
* [APIリファレンス](https://na-trium-144.github.io/webcface/namespaces.html)
* [webcface-python APIリファレンス](https://na-trium-144.github.io/webcface-python/)
* [webcface-js APIリファレンス](https://na-trium-144.github.io/webcface-js/)

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
* OpenCV (Apache 2.0) : https://opencv.org/license/
* ImageMagick: https://imagemagick.org/script/license.php
* tiny-process-library (MIT) : https://gitlab.com/eidheim/tiny-process-library (toolsで使用)
* toml++ (MIT) : https://github.com/marzer/tomlplusplus (toolsで使用)
