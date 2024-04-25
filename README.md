# WebCFace

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=C%2B%2B)](https://github.com/na-trium-144/webcface)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)
[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)  
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/webcface?logo=Python&logoColor=white)](https://github.com/na-trium-144/webcface-python)
[![PyPI - Version](https://img.shields.io/pypi/v/webcface)](https://pypi.org/project/webcface/)  
[![javascript](https://img.shields.io/badge/JavaScript%2C%20TypeScript-gray?logo=JavaScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
[![npm](https://img.shields.io/npm/v/webcface)](https://www.npmjs.com/package/webcface)

Web-based RPC &amp; UI Library

WebSocketとMessagePackを使った、ROSのような分散型の通信ライブラリです。
クロスプラットフォームかつ異なる言語間で数値、文字列、画像などのデータを送受信したり、関数(手続き)を呼び出したりすることができます。

WebCFaceはプログラム間の通信のライブラリとして使うことができるだけでなく、
WebブラウザーでアクセスできるUI(WebUI)を使うことでWebCFaceで通信されているデータを可視化したり関数を呼び出したりできます。
WebUIでは簡易なUIを作成したり2D、3Dの図形の描画をさせることもできます。

![webcface-webui](https://raw.githubusercontent.com/na-trium-144/webcface/main/docs/images/webcface-webui.png)

* [plotjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin) を使うと、WebCFaceで通信されているデータを [PlotJuggler](https://github.com/facontidavide/PlotJuggler) を使って見ることもできます。

## Benchmark

src/example/benchmark.cc で通信速度をチェックしてみました。
以下の表は クライアント→サーバー→クライアント でさまざまなサイズの文字列データの送受信にかかった時間です。

使用したPCのCPUは、MacOSは Apple M1 、それ以外は Intel Core i5-13500 です。

| OS | 10Byte | 100Byte | 1kByte | 10kByte | 100kByte | 1MByte |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| Windows (MSVC build) | 806 μs | 706 μs | 1.21 ms | 5.45 ms | 49.6 ms | 492 ms |
| Windows (MinGW build) | 15.6 ms | 15.5 ms | 15.5 ms | 15.6 ms | 15.6 ms | 28.4 ms |
| Server=MinGW + Client=MSVC | 620 μs | 576 μs | 619 μs | 908 μs | 4.73 ms | 42.0 ms |
| Server=MSVC + Client=MinGW | 15.6 ms | 15.0 ms | 15.3 ms | 16.1 ms | 55.5 ms | 432 ms |
| Linux (on WSL1) | 187 μs | 202 μs | 200 μs | 281 μs | 1.28 ms | 12.3 ms |
| Server=MinGW + Client=WSL1 | 207 μs | 207 μs | 273 μs | 399 μs | 1.73 ms | 17.3 ms |
| Server=MinGW + Client=WSL2 | 406 μs | 442 μs | 477 μs | 639 μs | 2.12 ms | 15.9 ms |
| Server=WSL1 + Client=MSVC | 703 μs | 750 μs | 769 μs | 1.01 ms | 4.61 ms | 38.7 ms |
| Server=WSL2 + Client=MSVC | 946 μs | 915 μs | 1.08 ms | 1.32 ms | 4.79 ms | 42.2 ms |

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

### Ubuntu 20.04, 22.04 (x86_64, arm64, armhf)
[WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からそれぞれ最新のdebパッケージをダウンロードしてインストールできます。

debパッケージはubuntu20.04と22.04でビルドしています。
それぞれ依存するパッケージのバージョンが違います。
Debianなど他のディストリビューションで動作するかはわかりません。

コマンドからダウンロードするなら以下のようにします。

<details open><summary>Ubuntu 22.04, x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.10.0/webcface_1.10.0-ubuntu22.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.3/webcface-tools_1.4.3-ubuntu22.04_amd64.deb
```
</details>

<details><summary>Ubuntu 22.04, arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.10.0/webcface_1.10.0-ubuntu22.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.3/webcface-tools_1.4.3-ubuntu22.04_arm64.deb
```
</details>

<details><summary>Ubuntu 22.04, armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.10.0/webcface_1.10.0-ubuntu22.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.3/webcface-tools_1.4.3-ubuntu22.04_armhf.deb
```
</details>

<details><summary>Ubuntu 20.04, x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.10.0/webcface_1.10.0-ubuntu20.04_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.3/webcface-tools_1.4.3-ubuntu20.04_amd64.deb
```
</details>

<details><summary>Ubuntu 20.04, arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.10.0/webcface_1.10.0-ubuntu20.04_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.3/webcface-tools_1.4.3-ubuntu20.04_arm64.deb
```
</details>

<details><summary>Ubuntu 20.04, armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v1.10.0/webcface_1.10.0-ubuntu20.04_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.6.0/webcface-webui_1.6.0-s_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v1.4.3/webcface-tools_1.4.3-ubuntu20.04_armhf.deb
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
[webcface-windows-packageのRelease](https://github.com/na-trium-144/webcface-windows-package/releases)からインストーラーをダウンロードしてください。

MinGW用バイナリは今のところ配布していません(ソースからビルドしてください)

### Build from source

以下はwebcfaceをソースからビルドする場合の説明です。(webcfaceをインストールした場合は不要です。)

#### Requirements
* c++20に対応したコンパイラが必要です
* テスト済みの環境
	* [![CMake Test (Linux GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml) (gcc-10以上)
	* [![CMake Test (Linux Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml) (clang-13以上)
	* [![CMake Test (MacOS Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml)
	* [![CMake Test (Windows MSVC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml)
	* [![CMake Test (Windows MinGW64 GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml)
* webcfaceは外部ライブラリとして [crow](https://github.com/CrowCpp/Crow), [asio](https://github.com/chriskohlhoff/asio), [libcurl](https://github.com/curl/curl), [eventpp](https://github.com/wqking/eventpp), [msgpack-cxx](https://github.com/msgpack/msgpack-c), [spdlog](https://github.com/gabime/spdlog), [cli11](https://github.com/CLIUtils/CLI11.git), [UTF8-CPP](https://github.com/nemtrif/utfcpp), [opencv](https://opencv.org/) を使用します。
	* cmake時に自動的にFetchContentでソースコード取得しビルドしますが、eventpp, msgpack, spdlog に関してはシステムにインストールされていてfind_packageで見つけることができればそれを使用します
	* opencvはソースからビルドしません。別途インストールする必要があります。
		* またはcmake時のオプションでopencvを使わないようにすることもできます (画像の変換機能が無効になります)
	* libcurlはwebsocket機能を有効にする必要があるためインストールされている場合でもソースからビルドします
	
<details><summary>Ubuntu 20.04, 22.04</summary>

```sh
sudo apt install build-essential git cmake
sudo apt install libopencv-dev libspdlog-dev  # optional
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
brew install opencv spdlog msgpack-cxx  # optional
```
</details>

<details><summary>Visual Studio</summary>

* Visual Studio 2019, 2022 でcloneしたwebcfaceのフォルダーを開くとビルドできます
* Developer Command Promptからcmakeコマンドを使ってもビルドできます
* OpenCVを使う場合は[公式サイト](https://opencv.org/releases/)からダウンロードしてください
	* またはchocolateyで`choco install opencv`など

</details>

<details><summary>MSYS2</summary>

```sh
pacman -S git mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
pacman -S mingw-w64-x86_64-opencv mingw-w64-x86_64-spdlog  # optional
```
</details>

#### Build (with Pure CMake)

```sh
cmake -Bbuild
cmake --build build
sudo cmake --build build -t install
```
* CMakeのオプション
	* `-DWEBCFACE_USE_OPENCV=off`にするとOpenCVを使用しないようになります
	* `-DWEBCFACE_FIND_LIBS=off`にするとmsgpack, eventpp, spdlogをfind_packageせずソースからビルドします
	* `-DWEBCFACE_SHARED=off`にすると共有ライブラリではなくすべて静的ライブラリになります
	* `-DWEBCFACE_EXAMPLE=on`でtestをビルドします(submoduleの場合デフォルトでoff)
	* `-DWEBCFACE_INSTALL=on`でtargetをinstallします(submoduleの場合デフォルトでoff)
		* さらに`-DWEBCFACE_INSTALL_SERVICE=on`で [webcface-server.service](cmake/webcafce-server.service) を lib/systemd/system にインストールします (デフォルトでoff)
	* `-DWEBCFACE_TEST=on`でtestをビルドします(デフォルトでoff)
		* テストが通らない場合テスト中の通信の待機時間を`-DWEBCFACE_TEST_TIMEOUT=100`などと伸ばすとうまく行く場合があります(デフォルト=10(ms))
	* `-DWEBCFACE_VERSION_SUFFIX`でバージョン表記を変更できます
		* 例えばバージョン(common/def.hに定義されている)が1.2.0のとき
		* `-DWEBCFACE_VERSION_SUFFIX=git` なら `git describe --tags` コマンドを使用して取得した文字列 (1.2.0-x-gxxxxxxx) になります(未指定の場合のデフォルト)
		* `git`以外の任意の文字列の場合 `-DWEBCFACE_VERSION_SUFFIX=hoge` で 1.2.0-hoge になります
		* `-DWEBCFACE_VERSION_SUFFIX=` で 1.2.0 だけになります

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
* Crow (BSD 3-Clause) : https://github.com/CrowCpp/Crow
* Asio (Boost Software License) : http://think-async.com/Asio/
* curl : https://curl.se/docs/copyright.html
* eventpp (Apache 2.0) : https://github.com/wqking/eventpp
* msgpack-c (Boost Software License) : https://github.com/msgpack/msgpack-c
* spdlog (MIT) : https://github.com/gabime/spdlog
* CLI11 (BSD 3-Clause) : https://github.com/CLIUtils/CLI11
* UTF8-CPP (BSD 1.0) : https://github.com/nemtrif/utfcpp
* OpenCV (Apache 2.0) : https://opencv.org/license/
* tiny-process-library (MIT) : https://gitlab.com/eidheim/tiny-process-library (toolsで使用)
* toml++ (MIT) : https://github.com/marzer/tomlplusplus (toolsで使用)
