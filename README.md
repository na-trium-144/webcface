# WebCFace

[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)

Web-based RPC &amp; UI Library

WebSocketとMessagePackを使った、ROSのような分散型の通信ライブラリです。
クロスプラットフォームかつ複数の言語間で通信ができます。

WebブラウザーでアクセスできるUI(webcface-webui)が付属しており、ネットワーク上のPCやタブレットなどからアクセスしてWebCFaceで通信されているデータを可視化したり関数を呼び出したりできます。
また、webuiからのアクセスを想定してテキストやボタンなどの配置を指定すると簡易なUIを作成することができます。

todo: ここにわかりやすいスクショを1つはる

todo: 機能の一覧をかんたんにかく

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
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.1.4/webcface_1.1.4_amd64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.5/webcface-webui_1.0.5_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.2/webcface-tools_1.1.2_amd64.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```

(webcface-toolsの内容と使い方についてはwebcface-toolsのReadmeを参照してください)

### Homebrew (MacOS, Linux)
```sh
brew tap na-trium-144/webcface
brew install webcface webcface-webui
```

### Build from source

* c++20に対応したコンパイラが必要です
* テスト済みの環境
	* [![CMake Test (Linux GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-gcc.yml) (gcc-10以上)
	* [![CMake Test (Linux Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-linux-clang.yml) (clang-13以上)
	* [![CMake Test (MacOS Clang)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-macos-clang.yml)
	* [![CMake Test (Windows MSVC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-msvc.yml)
	* [![CMake Test (Windows MinGW64 GCC)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml/badge.svg?branch=main)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-test-windows-gcc.yml)	(CMAKE_BUILD_TYPE=Debugだとリンクエラーになりました)
```sh
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
* その後、[webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードして webui/dist/ (installして使う場合は /path/to/prefix/share/webcface/dist) として展開してください
* webcfaceは外部ライブラリとして [cinatra](https://github.com/qicosmos/cinatra), [eventpp](https://github.com/wqking/eventpp), [msgpack-cxx](https://github.com/msgpack/msgpack-c), [spdlog](https://github.com/gabime/spdlog), [cli11](https://github.com/CLIUtils/CLI11.git) を使用します。
	* システムにインストールされてなければsubmoduleにあるソースコードをビルドしますが、eventpp, msgpack, spdlog に関してはインストールされていればそれを使用するのでビルドが速くなります
	* ubuntuなら `sudo apt install libspdlog-dev`, brewなら `brew install spdlog msgpack-cxx` でインストールできます
* このリポジトリのみでビルドしてinstallする代わりに、webcfaceを使いたいプロジェクトでこのリポジトリをsubmoduleとして追加して使うこともできます。

## Usage

チュートリアルと使い方→ [Documentation](https://na-trium-144.github.io/webcface/md_00__tutorial.html)
