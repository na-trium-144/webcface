# 3-2. Building from Source

\tableofcontents

以下はwebcfaceをソースからビルドする場合の説明です。(webcfaceをインストールした場合は不要です。)

\note
webcface ver1 をソースからビルドしたい場合は、[v1](https://github.com/na-trium-144/webcface/tree/v1) ブランチのREADMEを参照してください。

## Server & Library
### Requirements

* C++17に対応したコンパイラと、CMake, Meson(1.3.0以上), Ninja が必要です
    * ver1.11まではC++20が必要でしたが、ver2からC++17に移行しました
    * ビルドにはMesonを使用しますが、依存ライブラリにCMakeを使うものがあるのでそれもインストールする必要があります
    * MSVC以外では make と pkg-config も必要です
* Linuxはgcc-7以上とclang-7以上、MacはmacOS12(Monterey)以上、Visual Studioは2019以上でビルドできることを確認しています。
それ以前のバージョンでも動くかもしれません。
* MinGWでもビルドできます。MSYS2のUCRT64環境でテストしていますがMINGW64やCLANG64環境でもビルドできると思います。
* CygwinやMSYS2のMSYS環境ではasioがビルドできない([chriskohlhoff/asio#518](https://github.com/chriskohlhoff/asio/issues/518))ため、現状ではサーバー機能を除いてクライアントのみビルドすることができます。
    * asioはCygwin32bitでビルドできると主張しているが、32bitでもなぜかビルドできない<del>がissueを建てるのはめんどくさい</del>
    * 仮にビルドできたとしても、curlが使用するCygwinのsocketとasioが使用するWin32のsocketが干渉して動かない気もします

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
* デフォルトではインストールされたものは一切使わず、すべてソースをダウンロードしてビルドします。
    * 依存ライブラリをソースからビルドし、かつWebCFaceがsharedライブラリとしてビルドされる場合は、外部ライブラリのシンボルがWebCFace内部に隠されます。
    * そのためビルドしたWebCFaceを他の環境に配布する場合などはシステムにインストールしたライブラリを使用しないほうが良いです。
* しかし、オプションでインストールされたものを探して使うようにすることもできます。
    * 特にsharedまたはstaticライブラリとしてビルドできるspdlog,libcurl,Magick++は事前にインストールしておいたほうがビルドが高速になります。
    * システムにインストールされているものを使用した場合、ビルドしたWebCFaceはそのライブラリに依存することになります。
* libcurlはwebsocket機能を有効にする必要があるため、インストールされているlibcurlでwebsocketが使えない場合エラーになります。
* crowはunix_socketの機能が実装されている必要があるため、インストールされているcrowでunix_socketが使えない場合エラーになります。
* Magick++はマルチスレッドで実行するためにOpenMPが無効になっている必要があるため、インストールされているMagick++がOpenMPを使用してビルドされていた場合エラーになります、
* OpenCVはソースからビルドしません。OpenCVを使ったexampleをビルドしたい場合は別途インストールする必要がありますが、example以外では使用しないのでなくても問題ありません。

<details><summary>Ubuntu</summary>

```sh
sudo apt install build-essential git cmake pkg-config ninja-build
```
* ubuntu24.04
```sh
sudo apt install meson  # (only on 24.04)
```
* ubuntu22.04またはそれ以前ではaptでインストールできるmesonは古いので
```sh
sudo apt install python3-pip
pip install meson
```

```sh
# optional:
# sudo apt install libspdlog-dev libasio-dev
# sudo apt install libcli11-dev        # (only on 22.04 or later)
# sudo apt install libmsgpack-cxx-dev  # (only on 24.04 or later)
```

</details>

<details><summary>Homebrew (MacOS, Linux)</summary>

```sh
brew install cmake meson ninja
# optional:
# brew install msgpack-cxx spdlog asio cli11 utf8cpp
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

<!--
https://imagemagick.org/script/download.php からImageMagickをダウンロード、インストールしてPATHを通せばそれを使用してビルドすることができます。
(インストール時に development header もインストールすること)

または、chocolateyをインストールしてあれば `choco install imagemagick -PackageParameters InstallDevelopmentHeaders=true` でok
ImageMagickをインストールしない場合CMake時に自動的にソースをダウンロードしてビルドします。
-->
公式サイトからダウンロードできるImageMagickはOpenMPを使ってビルドされているため、インストールされていても使いません。
(またはImageMagickを別途 /noOpenMP オプション付きでビルドしたものを用意しPATHを通せばそれを使用することは可能です)

</details>

<details><summary>MSYS2 MinGW</summary>

```sh
pacman -S pactoys
pacboy -S git make gcc:p cmake:p ninja:p meson:p
# optional:
# pacboy -S msgpack-cxx:p spdlog:p asio:p cli11:p utf8cpp:p
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
* buildtypeを変更するには `--buildtype=release` または `--buildtype-debug` を指定してください
(WebCFaceがsubprojectでない場合デフォルトでrelease)
* staticライブラリをビルドするには `-Ddefault_library=static` を指定してください
    * `default_library=both` は現在非対応です
* `cpp_std`はc++17以上が必要です。またcygwinではgnu++17が必要です。
(WebCFaceがsubprojectでない場合デフォルトはgnu++17,c++17)
* `warning_level`は3以下であればビルドできるはずです
(WebCFaceがsubprojectでない場合デフォルトで3)。
    * todo: `warning_level=everything` でビルドできるかは未確認です。
* `-Dserver=disabled` でserverのビルドをオフ、 `enabled` でオンにできます
(デフォルト(auto)はWebCFaceがsubprojectでない and cygwinでない 場合のみenabledになります)
* `-Dexamples=disabled` でexampleのビルドをオフ、 `enabled` でオンにできます
(デフォルト(auto)はWebCFaceがsubprojectでない場合のみenabledになります)
    * `-Dcv_examples=disabled` にするとexamplesのうちOpenCVを使うもののみをオフにすることもできます
* `-Dtests=disabled` でexampleのビルドをオフ、 `enabled` でオンにできます
(デフォルト(auto)はWebCFaceがsubprojectでない場合のみenabledになります)
    * テストが通らない場合テスト中の通信の待機時間を `-Dtest_wait=100`などと伸ばすとうまく行く場合があります(デフォルト=10(ms))
* インストール先はデフォルトで /usr/local ですが、 `--prefix=~/.webcface` などと指定すると変更することができます
<!--
* `-Dinstall_service=true`で [webcface-server.service](cmake/webcafce-server.service) を lib/systemd/system にインストールします (デフォルトでOFF)
-->
* `-Dversion_suffix`でバージョン表記を変更できます
    * 例えばバージョンが1.2.0のとき
    * `-Dversion_suffix=git` なら `git describe --tags` コマンドを使用して取得した文字列 (1.2.0-x-gxxxxxxx) になります(未指定の場合のデフォルト)
    * `git`以外の任意の文字列の場合 `-Dversion_suffix=hoge` で 1.2.0-hoge になります
    * `-Dversion_suffix=` で 1.2.0 だけになります
* デフォルトでは最新のWebUIがbuildディレクトリにダウンロードされます。
`-Ddownload_webui=enabled` とするとダウンロードできなかった場合エラーになり、
`disabled` にするとダウンロードしません。
* WebCFaceがsubprojectでない場合、依存ライブラリはデフォルトではインストールされたものは一切使わず、すべてソースをダウンロードしてビルドします(`wrap_mode=forcefallback`)。
    * `-Dwrap_mode=default` とするとインストールされたものを探して使用します。
        * ただしlibcurl,Magick++,Crowはインストールされているものが使用できない場合エラーになるため、その場合は
        `"-Dforce_fallback_for=['libcurl','Magick++','Crow']"`
        などとして除外するとよいです。

setupが成功したら
```sh
meson compile -C build
```
でビルドします。
* Magick++-7.Q8 が見つからない などというエラーになった場合、そのまま再度compileをやり直すと成功する場合があります
(Mesonのバグ?)

```sh
meson test -C build
```
でテストを実行できます。

```sh
meson install -C build --skip-subprojects
```
でsetup時に指定したディレクトリにインストールできます。
(staticライブラリの場合は `--skip-subprojects` を使用せず、依存ライブラリもインストールした方がいいかも)

## WebUI

デフォルトではビルド済みのものがCMake時にダウンロードされますが、
`-Ddownload_webui=OFF` を指定した場合はダウンロードしません。
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
[OpenCV](https://opencv.org/),
[tiny-process-library](https://gitlab.com/eidheim/tiny-process-library),
[toml++](https://github.com/marzer/tomlplusplus)
を使用します。
システムにインストールされてなければOpenCV以外は自動的にソースからビルドします。  
OpenCVがインストールされていない場合は webcface-cv-capture がビルドされません。

