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
クロスプラットフォームかつ複数の言語間で通信ができます。

WebブラウザーでアクセスできるUI(webcface-webui)が付属しており、ネットワーク上のPCやタブレットなどからアクセスしてWebCFaceで通信されているデータを可視化したり関数を呼び出したりできます。
また、webuiからのアクセスを想定してテキストやボタンなどの配置を指定すると簡易なUIを作成することができます。

![webcface-webui](https://raw.githubusercontent.com/na-trium-144/webcface/main/docs/images/webcface-webui.png)

データ型を任意に定義できるROSとは違って、通信できるデータの種類が以下のように限定されています
* pub-sub通信
	* Value : 実数 or 実数の配列
	* Text : utf-8文字列
	* View : UIレイアウト
	* Log : 時刻とログレベルつきの文字列ストリーム
* その他
	* Func : 関数(他クライアントから引数とともに呼び出し、値を返す)

## Links

* [webcface](https://github.com/na-trium-144/webcface): サーバー & C++クライアントライブラリ (このリポジトリ)
* [webcface-webui](https://github.com/na-trium-144/webcface-webui): webブラウザ用UIアプリ
* [webcface-tools](https://github.com/na-trium-144/webcface-tools): コマンドラインツール群
* [webcface-js](https://github.com/na-trium-144/webcface-js): JavaScriptクライアントライブラリ
* [webcface-python](https://github.com/na-trium-144/webcface-python): Pythonクライアントライブラリ
* [homebrew-webcface](https://github.com/na-trium-144/homebrew-webcface): HomebrewのTapを管理しています
* [webcface-windows-package](https://github.com/na-trium-144/webcface-windows-package): Windows用ビルドをリリースする場所

## Installation
以下の手順で webcface, webcface-webui, webcface-tools をインストールできます。
(webcface-toolsの内容と使い方についてはwebcface-toolsのReadmeを参照してください)

### Ubuntu 20.04, 22.04 (x86_64, arm64, armhf)
[WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からそれぞれ最新のdebパッケージをダウンロードしてインストールできます。

debパッケージはubuntu20.04と22.04でビルドしています。
それぞれ依存するパッケージのバージョンが違います。
Debianなど他のディストリビューションで動作するかはわかりません。

コマンドからダウンロードするなら以下のようにします。

* Ubuntu 22.04, x86_64
```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.2.2/webcface_1.2.2-ubuntu22.04_amd64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.12/webcface-webui_1.0.12_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.7/webcface-tools_1.1.7-ubuntu22.04_amd64.deb
```

<details><summary>Ubuntu 22.04, arm64</summary>

```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.2.2/webcface_1.2.2-ubuntu22.04_arm64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.12/webcface-webui_1.0.12_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.7/webcface-tools_1.1.7-ubuntu22.04_arm64.deb
```
</details>

<details><summary>Ubuntu 22.04, armhf</summary>

```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.2.2/webcface_1.2.2-ubuntu22.04_armhf.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.12/webcface-webui_1.0.12_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.7/webcface-tools_1.1.7-ubuntu22.04_armhf.deb
```
</details>

<details><summary>Ubuntu 20.04, x86_64</summary>

```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.2.2/webcface_1.2.2-ubuntu20.04_amd64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.12/webcface-webui_1.0.12_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.7/webcface-tools_1.1.7-ubuntu20.04_amd64.deb
```
</details>

<details><summary>Ubuntu 20.04, arm64</summary>

```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.2.2/webcface_1.2.2-ubuntu20.04_arm64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.12/webcface-webui_1.0.12_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.7/webcface-tools_1.1.7-ubuntu20.04_arm64.deb
```
</details>

<details><summary>Ubuntu 20.04, armhf</summary>

```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.2.2/webcface_1.2.2-ubuntu20.04_armhf.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.12/webcface-webui_1.0.12_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.7/webcface-tools_1.1.7-ubuntu20.04_armhf.deb
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

### Windows
[webcface-windows-packageのRelease](https://github.com/na-trium-144/webcface-windows-package/releases)からインストーラーをダウンロードしてください。

### Build from source

#### Requirements
* c++20に対応したコンパイラが必要です
* テスト済みの環境
	* [![CMake Test (Linux GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml) (gcc-10以上)
	* [![CMake Test (Linux Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml) (clang-13以上)
	* [![CMake Test (MacOS Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml)
	* [![CMake Test (Windows MSVC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml)
	* [![CMake Test (Windows MinGW64 GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml)
* webcfaceは外部ライブラリとして [crow](https://github.com/CrowCpp/Crow), [asio](https://github.com/chriskohlhoff/asio), [libcurl](https://github.com/curl/curl), [eventpp](https://github.com/wqking/eventpp), [msgpack-cxx](https://github.com/msgpack/msgpack-c), [spdlog](https://github.com/gabime/spdlog), [cli11](https://github.com/CLIUtils/CLI11.git) を使用します。
	* cmake時に自動的にFetchContentでソースコード取得しビルドしますが、eventpp, msgpack, spdlog に関してはシステムにインストールされていてfind_packageで見つけることができればそれを使用します
	* libcurlはwebsocket機能を有効にする必要があるためインストールされている場合でもソースからビルドします
	* crowは暫定的にWindowsでSegmentation faultするバグを修正したもの([na-trium-144/Crow](https://github.com/na-trium-144/Crow) の fix-destructor-io-service ブランチ)を使用しています

<details><summary>Ubuntu 20.04, 22.04</summary>

```sh
sudo apt install build-essential git cmake
sudo apt install libspdlog-dev  # optional
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
brew install spdlog msgpack-cxx  # optional
```
</details>

<details><summary>Visual Studio</summary>

* Visual Studio 2019, 2022 でcloneしたwebcfaceのフォルダーを開くとビルドできます
* Developer Command Promptからcmakeコマンドを使ってもビルドできます
</details>

<details><summary>MSYS2</summary>

```sh
pacman -S git mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
pacman -S mingw-w64-x86_64-spdlog  # optional
```
</details>

#### Build (with Pure CMake)

```sh
cmake -Bbuild
cmake --build build
sudo cmake --build build -t install
```
* CMakeのオプション
	* `-DWEBCFACE_SHARED=off`にすると共有ライブラリではなくすべて静的ライブラリになります
	* `-DWEBCFACE_EXAMPLE=on`でtestをビルドします(submoduleの場合デフォルトでoff)
	* `-DWEBCFACE_INSTALL=on`でtergetのinstallをします(submoduleの場合デフォルトでoff)
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
* [Tutorial](https://na-trium-144.github.io/webcface/md_00__tutorial.html)
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
* tiny-process-library (MIT) : https://gitlab.com/eidheim/tiny-process-library (toolsで使用)
* toml++ (MIT) : https://github.com/marzer/tomlplusplus (toolsで使用)
