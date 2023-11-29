# WebCFace

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?logo=C%2B%2B)](https://github.com/na-trium-144/webcface)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)
[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)  
[![python](https://img.shields.io/badge/python-work_in_progress-red?logo=Python&logoColor=white)](https://github.com/na-trium-144/webcface-python/tree/dev)  
[![javascript](https://img.shields.io/badge/JavaScript-gray?logo=JavaScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
[![typescript](https://img.shields.io/badge/TypeScript-gray?logo=TypeScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
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
* [webcface-js](https://github.com/na-trium-144/webcface-js): JavaScriptクライアントライブラリ
* [webcface-python](https://github.com/na-trium-144/webcface-python): Pythonクライアントライブラリ
* [webcface-tools](https://github.com/na-trium-144/webcface-tools): コマンドラインツール群

## Installation

### Ubuntu 22.04 (amd64, arm64, armhf)
[WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からそれぞれ最新のdebパッケージをダウンロードしてインストールできます。

debパッケージはubuntu22.04でビルドしています。20.04以前のubuntuでは依存ライブラリの都合で動きません。debianはわかりません。

例 (amd64の場合)
```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.1.8/webcface_1.1.8_amd64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.7/webcface-webui_1.0.7_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.3/webcface-tools_1.1.3_amd64.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```

(webcface-toolsの内容と使い方についてはwebcface-toolsのReadmeを参照してください)

### Homebrew (MacOS, Linux)
```sh
brew tap na-trium-144/webcface
brew install webcface webcface-webui webcface-tools
```

### Build from source

#### Requirements
* c++20に対応したコンパイラが必要です
* テスト済みの環境
	* [![CMake Test (Linux GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml) (gcc-10以上)
	* [![CMake Test (Linux Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml) (clang-13以上)
	* [![CMake Test (MacOS Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml)
	* [![CMake Test (Windows MSVC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml)
	* [![CMake Test (Windows MinGW64 GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml)
* webcfaceは外部ライブラリとして [crow](https://github.com/CrowCpp/Crow), [libcurl](https://github.com/curl/curl), [eventpp](https://github.com/wqking/eventpp), [msgpack-cxx](https://github.com/msgpack/msgpack-c), [spdlog](https://github.com/gabime/spdlog), [cli11](https://github.com/CLIUtils/CLI11.git) を使用します。
	* システムにインストールされてなければsubmoduleにあるソースコードをビルドしますが、eventpp, msgpack, spdlog に関してはインストールされていればそれを使用します
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

* Visual Studio と Git for Windows をインストールし、Developer command prompt からビルドすればいいはずです
* インストール先は/usr/localではない
</details>

<details><summary>MSYS2</summary>

```sh
pacman -S git mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
pacman -S mingw-w64-x86_64-spdlog  # optional
```
</details>

#### Build

```sh
git clone https://github.com/na-trium-144/webcface
cd webcface
git submodule update --init --recursive
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
* その後、[webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードして /usr/local/share/webcface/dist として展開する必要があります
	* install先が/usr/localでない場合はprefixを読み替えてください
	* installしないでbuildディレクトリから起動する場合は、このリポジトリ直下にdist/を置いてください
	* コマンドからやる場合は次のようになります
```sh
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.7/webcface-webui_1.0.7.tar.gz
tar zxvf webcface-webui*.tar.gz
sudo rm -rf /usr/local/share/webcface
sudo mkdir /usr/local/share/webcface
sudo mv dist /usr/local/share/webcface/dist
rm webcface-webui*.tar.gz
```

* このリポジトリのみでビルドしてinstallする代わりに、webcfaceを使いたいプロジェクトでこのリポジトリをsubmoduleとして追加して使うこともできます。

## Usage
→ [Tutorial](https://na-trium-144.github.io/webcface/md_00__tutorial.html)
