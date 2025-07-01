# 3-2. Building from Source

\tableofcontents

以下はwebcfaceをソースからビルドする場合の説明です。(webcfaceをインストールした場合は不要です。)

\note
webcface ver1 をソースからビルドしたい場合は、[v1](https://github.com/na-trium-144/webcface/tree/v1) ブランチのREADMEを参照してください。

## Server & Library
### Requirements

* C++17に対応したコンパイラと、Meson(>=1.3.0)、Ninja が必要です
    * ver1.11まではC++20が必要でしたが、ver2からC++17に移行しました
    * インストール済みの依存ライブラリを使用するためには CMake や pkgcong (pkg-config) も必要になります
* Linux (x86_64, arm64, armhf) は gcc-9 以上と clang-7 以上、
Mac (x86_64, arm64) は macOS13 (Ventura) 以上、
Windowsは Visual Studio 2022 の x86, x64, arm64、
MinGWは MSYS2 の UCRT64 環境
でのビルドをテストしています。
それより前のバージョンでも動く可能性はあります。
* CygwinやMSYS2のMSYS環境ではasioがビルドできない([chriskohlhoff/asio#518](https://github.com/chriskohlhoff/asio/issues/518))ため、現状ではサーバー機能を除いてクライアントのみビルドすることができます。
    * asioはCygwin32bitでビルドできると主張しているが、32bitでもなぜかビルドできない<del>がissueを建てるのはめんどくさい</del>
    * 仮にビルドできたとしても、curlが使用するCygwinのsocketとasioが使用するWin32のsocketが干渉して動かない気もします

### Dependencies

<!-- subprojects/ 以下のファイル名順 -->
* webcfaceは外部ライブラリとして
[Asio](https://github.com/chriskohlhoff/asio),
[CLI11](https://github.com/CLIUtils/CLI11.git),
[Crow](https://github.com/CrowCpp/Crow),
[curl](https://github.com/curl/curl),
[fmt](https://github.com/fmtlib/fmt)(>=11),
[libvips](https://github.com/libvips/libvips),
[msgpack-cxx](https://github.com/msgpack/msgpack-c),
[spdlog](https://github.com/gabime/spdlog),
[UTF8-CPP](https://github.com/nemtrif/utfcpp),
[GoogleTest](https://github.com/google/googletest)(testのみ)
[OpenCV](https://opencv.org/)(exampleのみ),
を使用します。
* <del>デフォルトではインストールされたものは一切使わず、すべてソースをダウンロードしてビルドします。</del>
* <span class="since-c">2.6</span>
システムにインストールされているものがあればデフォルトでそれを探して使用します。
* しかしインストールされていない場合は自動的に必要なソースをダウンロードしてビルドするので、WebCFaceを使うためにこれらのライブラリをすべてインストールする必要はありません。
    * オプションでシステムにインストールされているものを探さず常にソースからビルドさせることもできます。
    時間はかかりますが、動作テスト済みのバージョンの組み合わせでビルドされるので確実性が上がります。
* これらの依存ライブラリをすべてソースからビルドし、かつWebCFaceがsharedライブラリとしてビルドされる場合は、依存ライブラリはstaticライブラリとしてWebCFace内部に埋め込まれ、シンボルも隠されます。
    * そのためビルドしたWebCFaceを他の環境に配布する場合などはシステムにインストールしたライブラリを使用しないほうが良いです。
* spdlogは `SPDLOG_FMT_EXTERNAL` が有効になっている必要があります。
またfmt>=11を使用するため、それより古いfmtを使ってビルドされているspdlogは使えません。
* libcurlはwebsocket機能を有効にする必要があるため、インストールされているlibcurlが古いもしくはwebsocketが無効になっている場合ビルド前にエラーになります。
* crowはunix_socketの機能が実装されている必要があるため、インストールされているcrowでunix_socketが使えない場合ビルド前にエラーになります。
    * 現在はこの機能はリリースされておらず、[PR#803](https://github.com/CrowCpp/Crow/pull/803) のコードが必要です。
* OpenCVはインストールされていない場合でもソースからビルドしません。OpenCVを使ったexampleをビルドしたい場合は別途インストールする必要がありますが、example以外では使用しないのでなくても問題ありません。

<details><summary>Ubuntu</summary>

```sh
sudo apt install build-essential git cmake pkg-config ninja-build
```
* ubuntu24.04
```sh
sudo apt install meson
```
* ubuntu22.04またはそれ以前ではaptでインストールできるmesonは古いので
```sh
sudo apt install python3-pip
pip install meson
```
* 依存ライブラリ(optional)
    * `libvips-dev` はwebsocketが無効の `libcurl4` に依存しているため使用できません。
    * `libspdlog-dev`, `libfmt-dev` は古いため使用できません。
```sh
# sudo apt install libasio-dev
# sudo apt install libcli11-dev        # (only on 22.04 or later)
# sudo apt install libmsgpack-cxx-dev  # (only on 24.04 or later)
```

</details>

<details><summary>Homebrew (MacOS, Linux)</summary>

```sh
brew install cmake meson ninja
```
* 依存ライブラリ(optional)
```
# brew install msgpack-cxx fmt spdlog asio cli11 utf8cpp vips curl
```

</details>

<details><summary>Visual Studio</summary>

* Visual Studio 2019 または 2022 をインストールしてください。
    * ImageMagickをソースからビルドするために Visual C++ ATL と MFC のコンポーネントも必要になります。
    * 2017でもビルドできるかもしれません(未確認)
* MesonとNinjaをインストールしてください。
    * https://github.com/mesonbuild/meson/releases からmsi形式でダウンロード、インストールできます
    (see also https://mesonbuild.com/Getting-meson.html)
    * または `choco install meson`
* Visual Studio の Developer Command Prompt からmesonコマンドを起動してください。

</details>

<details><summary>MSYS2 MinGW</summary>

```sh
pacman -S pactoys
pacboy -S git make gcc:p cmake:p ninja:p meson:p
```
* 依存ライブラリ(optional)
```sh
# pacboy -S msgpack-cxx:p fmt:p spdlog:p asio:p cli11:p utf8cpp:p vips:p
```

</details>

<details><summary>MSYS2 MSYS</summary>

```sh
pacman -S git make gcc cmake ninja meson
```

</details>

<details><summary>Cygwin</summary>

gcc-core, gcc-g++, cmake, make, meson, pkg-config, ninja をインストールしてください

</details>

### Build

```sh
git clone https://github.com/na-trium-144/webcface.git
cd webcface
meson setup build
```
* Visual Studio の場合 `--backend vs` を指定すると Visual Studio のプロジェクトファイルを生成します
* <span class="since-c">2.6</span> buildtypeはデフォルトでdebugです。変更するには `--buildtype=release` または `--buildtype=debug` を指定してください
* staticライブラリをビルドするには `-Ddefault_library=static` を指定してください
    * `default_library=both` は現在非対応です
* <span class="since-c">2.6</span> それぞれの依存ライブラリはまずシステムにインストールされているものを探し、見つからなければsubprojectにフォールバックします。
    * ただしlibcurlやCrowはインストールされているものが使用できない場合エラーになるため、その場合は
    `--force-fallback-for=libcurl,Crow`
    などとして除外する必要があります。
    * `-Dwrap_mode=forcefallback` とするとインストールされたものを探さずすべてソースビルドします。
    * インストールされているのに見つからない場合は環境変数の `PKG_CONFIG_PATH` か、引数で `-Dpkg_config_path` や `-Dcmake_prefix_path` などを設定してください
* `cpp_std`はc++17以上が必要です。またcygwinではgnu++17が必要です。
(WebCFaceがsubprojectでない場合デフォルトはgnu++17,c++17)
* `warning_level`は3以下であればビルドできるはずです
(WebCFaceがsubprojectでない場合デフォルトで3)。
    * `warning_level=everything` でビルドできるかは未確認です。
* `-Dserver=disabled` でserverのビルドをオフ、 `enabled` でオンにできます
(デフォルト(auto)はWebCFaceがsubprojectでない and cygwinでない 場合のみenabledになります)
* `-Dexamples=disabled` でexampleのビルドをオフ、 `enabled` でオンにできます
(デフォルト(auto)はWebCFaceがsubprojectでない場合のみenabledになります)
    * `-Dcv_examples=disabled` にするとexamplesのうちOpenCVを使うもののみをオフにすることもできます
* `-Dtests=disabled` でexampleのビルドをオフ、 `enabled` でオンにできます
(デフォルト(auto)はWebCFaceがsubprojectでない場合のみenabledになります)
    * テストが通らない場合テスト中の通信の待機時間を `-Dtest_wait=100`などと伸ばすとうまく行く場合があります(デフォルト=10(ms))
* インストール先はデフォルトで /usr/local ですが、 `--prefix=~/.webcface` などと指定すると変更することができます
* `-Dversion_suffix`でバージョン表記を変更できます
    * 例えばバージョンが1.2.0のとき
    * `-Dversion_suffix=git` なら `git describe --tags` コマンドを使用して取得した文字列 (1.2.0-x-gxxxxxxx) になります(未指定の場合のデフォルト)
    * `git`以外の任意の文字列の場合 `-Dversion_suffix=hoge` で 1.2.0-hoge になります
    * `-Dversion_suffix=` で 1.2.0 だけになります
* デフォルトでは最新のWebUIがbuildディレクトリにダウンロードされます。
`-Ddownload_webui=enabled` とするとダウンロードできなかった場合エラーになり、
`disabled` にするとダウンロードしません。

setupが成功したら
```sh
meson compile -C build
```
でビルドします。

```sh
meson test -C build --suite webcface
```
でテストを実行できます。

```sh
meson install -C build --skip-subprojects
```
でsetup時に指定したディレクトリにインストールできます。
(staticライブラリの場合は `--skip-subprojects` を使用せず、依存ライブラリもインストールした方がいいかも)

## WebUI

デフォルトではビルド済みのものが meson setup 時にダウンロードされますが、
`-Ddownload_webui=disabled` を指定した場合はダウンロードしません。
その場合は [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) からビルド済みのtar.gzのアーカイブをダウンロードしてください。

インストールする場合は (webcfaceのインストール場所)/share/webcface/dist として展開してください。  
installせずに実行する場合は webcface-server のバイナリと同じディレクトリか、その1, 2, 3階層上のどこかにdistディレクトリを配置してください。

Ubuntuの場合は[webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases)にあるdebパッケージで、またhomebrewでは `webcface-webui` パッケージとしてWebUIだけを単体でインストールすることもできます。

または自分でビルドすることも可能です。(node.jsが必要)
[webcface-webui](https://github.com/na-trium-144/webcface-webui) のREADMEを参照してください。

## tools

https://github.com/na-trium-144/webcface-tools.git をcloneしてビルド、インストールします。
WebCFaceライブラリのビルドと同様、Mesonを使って
```sh
meson setup build
meson compile -Cbuild
meson install -C build --skip-subprojects
```
のようにビルド、インストールできます。

toolsのビルド時にWebCFaceライブラリが見つからないとなる場合は
環境変数`PKG_CONFIG_PATH`(またはmesonの引数`-Dpkg_config_path`)に (webcfaceのパス)/lib/pkgconfig を追加するか、
環境変数`PATH`に (webcfaceのパス)/bin を追加するか、
mesonの引数`-Dcmake_prefix_path`にwebcfaceのパスを追加するなどすればビルドできるはずです。

webcface-toolsは外部ライブラリとして
[spdlog](https://github.com/gabime/spdlog),
[cli11](https://github.com/CLIUtils/CLI11.git),
[FTXUI](https://github.com/ArthurSonzogni/FTXUI),
[tiny-process-library](https://gitlab.com/eidheim/tiny-process-library),
[toml++](https://github.com/marzer/tomlplusplus)
を使用します。
システムにインストールされてなければ自動的にソースからビルドします。  

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [3-1. Setup WebCFace Library](31_setup.md) | [4-1. Client](41_client.md) |

</div>
