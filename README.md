# webcface

[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)

Web-based RPC &amp; UI Library

ここに説明を書く

## Example

例を示す

## Installation

### Debian, Ubuntu (amd64, arm64, armv7)
[WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からそれぞれ最新のdebパッケージをダウンロードしてインストールできます。

例
```bash
curl -LO https://github.com/na-trium-144/webcface/releases/download/v1.0.0/webcface_1.0.0_amd64.deb
curl -LO https://github.com/na-trium-144/webcface-webui/releases/download/v1.0.0/webcface-webui_1.0.0_all.deb
sudo apt install ./webcface*.deb
```

### homebrew (MacOS, Linux)
```bash
brew tap na-trium-144/webcface
brew install webcface webcface-webui
```

### ソースからビルドする
このリポジトリをcloneして
```bash
git submodule update --init --recursive
cmake -Bbuild
cmake --build build
sudo cmake --build build -t install # installする場合
```
```bash
cd webui
npm ci
npm run build
# installする場合は
sudo cp -r dist /usr/local/share/webcface/dist  # または /path/to/prefix/share/webcface/dist
```
* c++20に対応したコンパイラ & node.js v18以上 が必要です
	* 後者は[webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードして webui/dist/ (installする場合は /path/to/prefix/share/webcface/dist) として展開することで代用できます
* webcfaceは外部ライブラリとして [cinatra](https://github.com/qicosmos/cinatra), [eventpp](https://github.com/wqking/eventpp), [msgpack-cxx](https://github.com/msgpack/msgpack-c), [spdlog](https://github.com/gabime/spdlog), [tclap](https://tclap.sourceforge.net) を使用します。
	* eventpp, msgpack, spdlog に関してはシステムにインストールされてなければsubmoduleにあるソースコードをビルドしますが、インストールされていればそれを使用するのでビルドが速くなります
	* ubuntuなら `sudo apt install libspdlog-dev`, brewなら `brew install spdlog msgpack-cxx` でインストールできます
* `-DWEBCFACE_EXAMPLE=on`でexampleを、`-DWEBCFACE_TEST=on`でtestをビルドします。

## Usage

### Server
WebCFaceを使用するときはserverを常時立ち上げておく必要があります

```bash
webcface-server
```
でサーバーを起動します
* ソースからビルドした場合は `build/webcface-server`

起動時に表示されるurl (http://pcのipアドレス:7530/) をブラウザから開くとデータにアクセスすることができます

### Client (C++)

```cmake
find_package(webcface)
target_link_libraries(target PRIVATE webcface::webcface)
```

```c++
#include <webcface/webcface.h>

WebCFace::Client wcli("name of this client program");
```

Clientの使い方は [ここにリンクを貼る] を参照
