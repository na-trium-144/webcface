# WebCFace

[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)

Web-based RPC &amp; UI Library

C++とJavaScriptで使える、WebSocketを使ったプロセス間通信ライブラリです。
データの送受信だけでなく、プロセス間での関数呼び出しができます。

また、WebブラウザーでアクセスできるUI(webcface-webui)から通信されているデータを確認したり関数を実行したりできる他、テキストやボタンなどを自由に配置してそのWebブラウザーに表示させることができます。

todo: ここにわかりやすいスクショを1つはる
todo: 機能の一覧をかんたんにかく

## Related Links

* webcface: サーバー & C++クライアント
* [webcface-webui](https://github.com/na-trium-144/webcface-webui): webブラウザ用UIアプリ
* [webcface-js](https://github.com/na-trium-144/webcface-js): JavaScriptクライアント
* [webcface-tools](https://github.com/na-trium-144/webcface-tools): クライアントとなるコマンド群

## Installation

### Debian, Ubuntu (amd64, arm64, armhf)
[WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からそれぞれ最新のdebパッケージをダウンロードしてインストールできます。

例 (amd64の場合)
```sh
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.1.2/webcface_1.1.2_amd64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.4/webcface-webui_1.0.4_all.deb
curl -LO https://github.com/na-trium-144/webcface-tools/releases/download/v1.1.0/webcface-tools_1.1.0_amd64.deb
sudo apt install ./webcface*.deb
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
	* Windows
		* MSVC: ok
		* MinGW Clang17
			* 終了時にSegmentation Faultするバグがありますが動作はします ([#43](https://github.com/na-trium-144/webcface/issues/43))
		* MinGW GCC13: NG (リンクエラー)
			* GCC12では動くっぽい?
	* Linux
		* GCC, Clang: ok
	* MacOS
		* Clang: ok

```sh
git submodule update --init --recursive
cmake -Bbuild
cmake --build build
sudo cmake --build build -t install
```
* その後、[webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードして webui/dist/ (installして使う場合は /path/to/prefix/share/webcface/dist) として展開してください
* webcfaceは外部ライブラリとして [cinatra](https://github.com/qicosmos/cinatra), [eventpp](https://github.com/wqking/eventpp), [msgpack-cxx](https://github.com/msgpack/msgpack-c), [spdlog](https://github.com/gabime/spdlog), [cli11](https://github.com/CLIUtils/CLI11.git) を使用します。
	* システムにインストールされてなければsubmoduleにあるソースコードをビルドしますが、eventpp, msgpack, spdlog に関してはインストールされていればそれを使用するのでビルドが速くなります
	* ubuntuなら `sudo apt install libspdlog-dev`, brewなら `brew install spdlog msgpack-cxx` でインストールできます
* `-DWEBCFACE_EXAMPLE=on`でexampleを、`-DWEBCFACE_TEST=on`でtestをビルドします。

## Usage

チュートリアルと使い方→ [Documentation](https://na-trium-144.github.io/webcface/md_00__tutorial.html)
