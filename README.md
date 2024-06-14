# WebCFace

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=C%2B%2B)](https://github.com/na-trium-144/webcface)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)
[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)  
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/webcface?logo=Python&logoColor=white)](https://github.com/na-trium-144/webcface-python)
[![PyPI - Version](https://img.shields.io/pypi/v/webcface)](https://pypi.org/project/webcface/)  
[![javascript](https://img.shields.io/badge/JavaScript%2C%20TypeScript-gray?logo=JavaScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
[![npm](https://img.shields.io/npm/v/webcface)](https://www.npmjs.com/package/webcface)

Web-based IPC &amp; Dashboard-like UI

WebSocketとMessagePackを使った、ROSのような分散型の通信ライブラリです。

C++ (C++20以上), C (C99), Python (3.8以上), JavaScript/TypeScript で相互に数値、文字列、画像などのデータを送受信したり、関数(手続き)を呼び出したりすることができます。
少し難易度は上がりますがCのAPIを経由することで他の言語からも使用できると思います。

Linux, Windows, MacOS で動作します。
WebSocketを使用しているため、Wi-FiやEtherNet経由で複数のPC間(OS問わず)で通信することも可能です。
WindowsとWSL1/2の間の相互通信も自動的に接続されます。

さらに同一マシン上やDocker,WSL経由など使用可能な場合はTCPの代わりにUnixドメインソケットを使用するようにし、パフォーマンスが改善しました。

## Features

### WebUI

WebCFaceではプログラム間でデータの送受信ができるAPIだけでなく、
WebブラウザーからWebCFaceで通信されているデータを可視化したり関数を呼び出したりできるUI(WebUI)を提供します。

さらにボタンや入力欄などの並べ方(View)をWebCFaceを使ったC++などのプログラムの側で定義してそれをWebUIに表示させることができ、
これによりHTMLやCSSの知識がなくても簡易なUIを作成することができます。

また、同様に2D、3Dの図形もWebCFaceを使ったプログラム側の記述のみでWebUIに描画させることができます。

![webcface-webui](https://raw.githubusercontent.com/na-trium-144/webcface/main/docs/images/webcface-webui.png)

なお、これらの描画データは View, Canvas2D, Canvas3D として他のデータ型(数値や文字列など)と同様にWebCFace内の通信データとして存在しており、
WebUI以外でもこれらのデータを受信して表示するアプリを作成することは可能です。

### PlotJuggler

[plotjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin) を使うと、WebCFaceで通信されているデータを [PlotJuggler](https://github.com/facontidavide/PlotJuggler) を使って見ることもできます。

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
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu24.04_amd64.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu24.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu24.04_arm64.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu24.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu24.04_armhf.deb
```
</details>

#### Ubuntu 22.04 Jammy
<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu22.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu22.04_amd64.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu22.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu22.04_arm64.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu22.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu22.04_armhf.deb
```
</details>

#### Ubuntu 20.04 Focal
<details><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu20.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu20.04_amd64.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu20.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu20.04_arm64.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.11.4/webcface_1.11.4-ubuntu20.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.4/webcface-tools_1.4.4-ubuntu20.04_armhf.deb
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

(ver1.12〜) Windows版WebCFaceのReleaseに含まれるビルド済みのspdlogは
`SPDLOG_WCHAR_SUPPORT` オプションがオンの状態です

MinGW用バイナリは今のところ配布していません(ソースからビルドしてください)

### Build from source

以下はwebcfaceをソースからビルドする場合の説明です。(webcfaceをインストールした場合は不要です。)

#### Requirements
* C++20に対応したコンパイラが必要です
	* GCCはgcc-10以上が必要です。
	* Clangはclang-13以上が必要です。
	* MacOSではMacOS12(Monterey)以上でビルドできることを確認済みです。
	* Visual Studio は2019以上でビルドできるはずです。
	* MinGWでもビルドできます。MSYS2のMINGW64環境でテストしていますがUCRT64やCLANG64環境でもビルドできると思います。
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
	* いずれもCMake時に自動的にFetchContentでソースコードを取得してビルドするので、必ずしもこれらをインストールする必要はありません。
	* eventpp, msgpack, spdlog, ImageMagick に関してはシステムにインストールされていてfind_packageで見つけることができればそれを使用します
	* OpenCVはソースからビルドしません。OpenCVを使ったexampleをビルドしたい場合は別途インストールする必要がありますが、example以外では使用しないのでほぼ必要ないと思います。
	* libcurlはwebsocket機能を有効にする必要があるためインストールされている場合でもソースからビルドします
	* googletestはchar8_tの機能を有効にする必要があるためインストールされている場合でもソースからビルドします
	
<details><summary>Ubuntu</summary>

```sh
sudo apt install build-essential git cmake
# optional:
sudo apt install libspdlog-dev libasio-dev libmagick++-dev
sudo apt install libcli11-dev # only on 22.04 or later
sudo apt install libmsgpack-cxx-dev # only on 24.04 or later
```

ubuntu20.04の場合デフォルトのコンパイラ(gcc-9)ではビルドできないのでgcc-10にする必要があります
```sh
sudo apt install gcc-10 g++-10
export CC=gcc-10
export CXX=g++-10
```
</details>

<details><summary>Homebrew (MacOS, Linux)</summary>

```sh
brew install cmake
# optional:
brew install msgpack-cxx spdlog asio cli11 utf8cpp imagemagick
```
</details>

<details><summary>Visual Studio</summary>

Visual Studio 2019, 2022 でcloneしたwebcfaceのフォルダーを開くか、
Developer Command Promptからcmakeコマンドを使ってもビルドできます

https://imagemagick.org/script/download.php からImageMagickをダウンロード、インストールしてPATHを通せばそれを使用してビルドすることができます。
(インストール時に development header もインストールすること)
インストールしない場合ソースをダウンロードしてビルドするので時間がかかります。

または、chocolateyをインストールしてあれば `choco install imagemagick -PackageParameters InstallDevelopmentHeaders=true` でok

</details>

<details><summary>MSYS2</summary>

```sh
pacman -S pactoys
pacboy -S git make gcc:p cmake:p ninja:p
# optional:
pacboy -S msgpack-cxx:p spdlog:p asio:p cli11:p utf8cpp:p imagemagick:p 
```
imagemagickをソースからビルドする際にninjaではなくmakeが必要になります

</details>

#### Build (with Pure CMake)

```sh
cmake -Bbuild
cmake --build build
sudo cmake --build build -t install
```
* CMakeのオプション
	* `-DWEBCFACE_SHARED=off`にすると共有ライブラリではなくすべて静的ライブラリになります
		* `-DWEBCFACE_PIC=on`または`-DCMAKE_POSITION_INDEPENDENT_CODE=on`にすると-fPICフラグが有効になります (Linux,Macのみ、WEBCFACE_SHAREDがonの場合on)
	* `-DWEBCFACE_EXAMPLE=on`でexampleをビルドします(submoduleの場合デフォルトでoff)
	* `-DWEBCFACE_INSTALL=on`でtargetをinstallします(submoduleの場合デフォルトでoff)
		* さらに`-DWEBCFACE_INSTALL_SERVICE=on`で [webcface-server.service](cmake/webcafce-server.service) を lib/systemd/system にインストールします (デフォルトでoff)
	* `-DWEBCFACE_TEST=on`でtestをビルドします(デフォルトでoff)
		* テストが通らない場合テスト中の通信の待機時間を`-DWEBCFACE_TEST_TIMEOUT=100`などと伸ばすとうまく行く場合があります(デフォルト=10(ms))
	* `-DWEBCFACE_VERSION_SUFFIX`でバージョン表記を変更できます
		* 例えばバージョン(common/def.hに定義されている)が1.2.0のとき
		* `-DWEBCFACE_VERSION_SUFFIX=git` なら `git describe --tags` コマンドを使用して取得した文字列 (1.2.0-x-gxxxxxxx) になります(未指定の場合のデフォルト)
		* `git`以外の任意の文字列の場合 `-DWEBCFACE_VERSION_SUFFIX=hoge` で 1.2.0-hoge になります
		* `-DWEBCFACE_VERSION_SUFFIX=` で 1.2.0 だけになります
	* `-DWEBCFACE_DOWNLOAD_WEBUI=off`を指定するとWebUIをダウンロードしません。
	* 依存ライブラリ
		* デフォルトではfind_packageやpkg_check_modulesなどで依存ライブラリがインストールされているか確認し、見つかればそれを使い見つからなければソースコードをダウンロードします。
		* `-DWEBCFACE_FIND_(ライブラリ)=off` にするとインストールしたものは使わず常にソースからダウンロードするようになります。
		* 設定可能なライブラリ名は以下
			* `MSGPACK`, `SPDLOG`, `EVENTPP`, `CURL`, `CROW`, `ASIO`, `CLI11`, `UTF8CPP`, `MAGICK`
			* `OPENCV` (デフォルトでoff、見つからなかった場合ソースからのビルドもしません)
			* Magickをソースビルドする場合のみ: `JPEG`, `PNG`, `ZLIB`, `WEBP`
		* `-DWEBCFACE_FIND_LIBS=off` とすると上記設定をすべてoffにします
			* WebCFaceと依存ライブラリをstaticライブラリとしてリンクしたい場合必要かも
			* すべてoffにしても依存ライブラリの内部でfind_packageが呼ばれるのは止められません (curl内部でopensslを参照するなど)
		* Windows(MSVC)でImageMagickをソースからビルドする場合、デフォルトでは`CMAKE_BUILD_TYPE`に指定したconfigurationのみビルドされます
			* DebugとReleaseの両方をビルドしたい場合は `-DWEBCFACE_CONFIG_ALL=on` を指定してください
		* spdlogのオプション
			* Windowsでは`SPDLOG_WCHAR_SUPPORT`がデフォルトでONになります
			* それ以外のオプション(SPDLOG_WCHAR_FILENAMES, SPDLOG_WCHAR_CONSOLE)はWebCFace内では設定しませんがコマンドラインオプションで指定することは可能です

#### Build (with colcon, ROS2)
* このリポジトリをワークスペースのsrcに追加して、colconでビルドすることができます

#### WebUI
* デフォルトではビルド済みのものがcmake時にダウンロードされます。
* `-DWEBCFACE_DOWNLOAD_WEBUI=off`を指定するとダウンロードしません。
	* その場合は [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードして /usr/local/share/webcface/dist として展開するのが簡単です。
		* install先が/usr/localでない場合はprefixを読み替えてください
		* installせずに実行する場合は webcface-server のバイナリと同じディレクトリか、その1, 2, 3階層上のどこかにdistディレクトリを配置してください
	* または自分でビルドすることも可能です(node.jsが必要)
* このリポジトリのReleasesにあるdebパッケージとhomebrewではwebcfaceのパッケージとは別で配布しています

#### tools
* toolsは別途 https://github.com/na-trium-144/webcface-tools.git をcloneしてビルド、インストールしてください

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
