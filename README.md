<sub><img src="https://raw.githubusercontent.com/na-trium-144/webcface-webui/main/public/icon.svg" height="40" /></sub>
WebCFace
====
<!-- ↑ a workaround to ensure the heading is properly displayed in both Doxygen and GitHub -->

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=C%2B%2B)](https://github.com/na-trium-144/webcface)
[![release](https://img.shields.io/github/v/release/na-trium-144/webcface)](https://github.com/na-trium-144/webcface/releases)
[![coverage](https://raw.githubusercontent.com/na-trium-144/webcface/badge/coverage.svg)](https://github.com/na-trium-144/webcface/actions/workflows/cmake-coverage.yml)  
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/webcface?logo=Python&logoColor=white)](https://github.com/na-trium-144/webcface-python)
[![PyPI - Version](https://img.shields.io/pypi/v/webcface)](https://pypi.org/project/webcface/)  
[![javascript](https://img.shields.io/badge/JavaScript%2C%20TypeScript-gray?logo=JavaScript&logoColor=white)](https://github.com/na-trium-144/webcface-js)
[![npm](https://img.shields.io/npm/v/webcface)](https://www.npmjs.com/package/webcface)

Web-based Communication Framework &amp; Dashboard-like UI

* WebCFace allows inter-process communication like ROS1, data visualization and function calls via GUI, and the creation of simple UIs in Immediate-Mode.
* It allows sending and receiving data such as numbers, strings, and images, as well as calling functions between C++ (C++17 or later), C, Python (3.6 or later), and JavaScript/TypeScript.
* Works on Linux, MacOS, and Windows (MSVC, MinGW, MSYS2, Cygwin).

> 
> * ROS1のようなプロセス間通信と、GUIによるデータの可視化や関数呼び出し、また Immediate-Mode でのシンプルなUIの作成ができます。
> * C++ (C++17以上), C, Python (3.6以上), JavaScript/TypeScript で相互に数値、文字列、画像などのデータを送受信したり、関数を呼び出したりすることができます。
> * Linux, MacOS, Windows(MSVC, MinGW, MSYS2, Cygwin) で動作します。

## Table of contents

<!--ts-->
   * [Table of contents](#table-of-contents)
   * [Features](#features)
      * [Easy to Setup](#easy-to-setup)
      * [Inter-Process Communication](#inter-process-communication)
      * [WebUI](#webui)
      * [WebCFace-Tools](#webcface-tools)
   * [Documentation](#documentation)
   * [Links](#links)
   * [Installation](#installation)
      * [Linux (x86_64, arm64, armhf) / MacOS (x86_64, arm64)](#linux-x86_64-arm64-armhf--macos-x86_64-arm64)
         * [Deb Package](#deb-package)
         * [Unzip manually (Linux)](#unzip-manually-linux)
         * [macOS](#macos)
      * [Homebrew (MacOS, Linux)](#homebrew-macos-linux)
      * [Windows MSVC (x86, x64)](#windows-msvc-x86-x64)
      * [Docker (x86_64, arm64, armhf)](#docker-x86_64-arm64-armhf)
         * [webcface-server](#webcface-server)
         * [webcface-tools](#webcface-tools-1)
   * [License](#license)
<!--te-->

## Features

### Easy to Setup

WebCFace consists of a server-side program `webcface-server` and client libraries.
To use it, simply start the server and run a program using the client library.
Unlike ROS, it does not have workspace management features, making it easy to integrate into any framework project.

For C / C++, if you are using CMake, you can use `find_package(webcface)`, or if you are using pkg-config, you can use `pkg-config --cflags --libs webcface`.

The library itself is a single file:

* Linux: `libwebcface.so.<version>`
* Mac: `libwebcface.<version>.dylib`
* Windows: `webcface-<version>.dll` (Release) or `webcfaced-<version>.dll` (Debug)

so you can also manually link to this library.

For Python and JavaScript, simply install the `webcface` package from PyPI / npm.
Since it uses WebSocket for communication, it works directly in web browsers.

> WebCFaceはサーバー側のプログラム `webcface-server` と、
> クライアントライブラリで構成されています。
> 使い方はサーバーを起動し、クライアントライブラリを利用したプログラムを起動するだけです。
> ROSのようなワークスペース管理機能はなく、どんなフレームワークのプロジェクトにも簡単に組み込むことができます。
> 
> C / C++ の場合、WebCFaceのライブラリは
> CMake を使っていれば `find_package(webcface)`、
> pkg-config なら`pkg-config --cflags --libs webcface`
> で簡単に利用できます。
> 
> またライブラリ本体は
> 
> * Linux: `libwebcface.so.<version>`
> * Mac: `libwebcface.<version>.dylib`
> * Windows: `webcface-<version>.dll` (Release) or `webcfaced-<version>.dll` (Debug)
> 
> の1つのみであり、手動でこのライブラリにリンクして使うこともできます。
> 
> Python, JavaScript には PyPI / npm に `webcface` パッケージを用意しているのでそれをインストールするだけで使えます。
通信にWebSocketを使用しているため、Webブラウザ上でもそのまま動作します。

### Inter-Process Communication

WebCFace uses WebSocket and MessagePack for communication.
This allows communication not only between processes but also with web browsers.
Additionally, if available, it uses Unix domain sockets instead of TCP for communication on the same machine or via Docker, WSL, etc.

The data types that can be sent and received with WebCFace include:

* Numeric type / Numeric array type (Value)
* String type (Text)
* Image (Image)
* Function call (Func)
* Text log (Log)

Although it may have less flexibility compared to ROS or gRPC, which allow users to define message types, it is easy to send and receive data with these combinations of data types.
For code examples, refer to the documentation [1-2. Tutorial (Communication)](https://na-trium-144.github.io/webcface/md_docs_212__tutorial__comm.html).

Image data can be resized or compressed to JPEG or PNG during transmission on the server side, reducing communication volume if compressed images are sufficient for display purposes.

The communication data format of WebCFace is common regardless of OS or library language, and it is backward compatible between versions.
This means that communication is possible between clients of different versions and different OS versions without any issues, as long as the server is newer than the client.
For details, refer to [8-4. Versioning](https://na-trium-144.github.io/webcface/md_docs_284__versioning.html).

> WebCFaceの通信にはWebSocketとMessagePackを使っています。
このためプロセス間だけでなくWebブラウザーとの通信が可能になっています。
さらに同一マシン上やDocker,WSL経由など使用可能な場合はTCPの代わりにUnixドメインソケットを使用します。
> 
> WebCFaceで送受信できるデータ型として
> 
> * 数値型・数値配列型(Value)
> * 文字列型(Text)
> * 画像(Image)
> * 関数呼び出し(Func)
> * テキストログ(Log)
> 
> などの型が用意されています。
> ユーザーがメッセージ型を定義できるROSやgRPCと比べると自由度は低いかもしれませんが、
> これらのデータ型の組み合わせであれば簡単に送受信させることができます。
> コード例についてはドキュメントの
> [1-2. Tutorial (Communication)](https://na-trium-144.github.io/webcface/md_docs_212__tutorial__comm.html)
> を参照してください。
> 
> Image型データは送受信の過程で画像を縮小したりJPEGやPNGに圧縮したりといった操作をサーバー側で行うことができます。
> 表示目的など、圧縮した画像で十分な場合には簡単に通信量を削減できます。
> 
> WebCFaceの通信データ形式はOSやライブラリの言語によらず共通で、またバージョン間で後方互換性があります。
> つまり、サーバーがクライアントより新しいバージョンでさえあれば、異なるバージョンのクライアント同士でも、異なるバージョンのOSでも問題なく通信が可能です。
> 詳細は
> [8-4. Versioning](https://na-trium-144.github.io/webcface/md_docs_284__versioning.html)
> を参照してください。

### WebUI

WebCFace provides not only an API for sending and receiving data between programs but also a UI (WebUI) that allows visualizing data and calling functions communicated via WebCFace from a web browser.

Furthermore, you can define the arrangement of buttons and input fields on the side of programs using WebCFace in C++, Python, etc., and display them in the WebUI.
This allows you to create simple UIs without knowledge of HTML or CSS.
For code examples, refer to the documentation [1-1. Tutorial (Visualizing)](https://na-trium-144.github.io/webcface/md_docs_211__tutorial__vis.html).

Similarly, 2D and 3D shapes can also be drawn in the WebUI with only descriptions on the side of programs using WebCFace.

> WebCFaceではプログラム間でデータの送受信ができるAPIだけでなく、
> WebブラウザーからWebCFaceで通信されているデータを可視化したり関数を呼び出したりできるUI(WebUI)を提供します。
> 
> さらにボタンや入力欄などの並べ方をWebCFaceを使ったC++,Pythonなどのプログラムの側で定義してそれをWebUIに表示させることができ、
> これによりHTMLやCSSの知識がなくても簡易なUIを作成することができます。
> コード例はドキュメントの
> [1-1. Tutorial (Visualizing)](https://na-trium-144.github.io/webcface/md_docs_211__tutorial__vis.html)
> を参照してください。
> 
> また、同様に2D、3Dの図形もWebCFaceを使ったプログラム側の記述のみでWebUIに描画させることができます。

![webcface-webui](https://raw.githubusercontent.com/na-trium-144/webcface/main/docs/images/webcface-webui.png)

### WebCFace-Tools

Several commands are provided to access WebCFace data from the command line.

[webcface-launcher](https://na-trium-144.github.io/webcface/md_docs_271__launcher.html)
is a feature that allows you to start and stop pre-registered commands from the WebCFace View.
You can keep only the server and launcher running at all times and remotely control programs using the WebUI.

[webcface-tui](https://na-trium-144.github.io/webcface/md_docs_275__tui.html)
is a TUI application that can be operated on the terminal, allowing you to check data in real-time and operate the View without opening a web browser.

> コマンドラインからWebCFaceのデータにアクセスできるコマンドもいくつか用意しています。
> 
> [webcface-launcher](https://na-trium-144.github.io/webcface/md_docs_271__launcher.html)
> は事前に登録しておいたコマンドの起動・停止をWebCFaceのViewから操作することができる機能です。
> serverとlauncherだけを常時起動しておき、WebUIを使ってプログラムをリモートに操作するという使い方ができます。
> 
> [webcface-tui](https://na-trium-144.github.io/webcface/md_docs_275__tui.html)
> はターミナル上で操作できるTUIアプリで、Webブラウザを開かなくてもデータをリアルタイムで確認したり、Viewの操作もできます。

![tui_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tui_value.gif)

![tui_view](https://github.com/na-trium-144/webcface/raw/main/docs/images/tui_view.png)

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

## Documentation

Currently available in Japanese only.

* Download and installation instructions are in the README (below this)
* 1 Tutorial: [Visualizing](https://na-trium-144.github.io/webcface/md_docs_211__tutorial__vis.html) / [Communication](https://na-trium-144.github.io/webcface/md_docs_212__tutorial__comm.html)
* 2 [Server](https://na-trium-144.github.io/webcface/md_docs_221__server.html) / [WebUI](https://na-trium-144.github.io/webcface/md_docs_222__webui.html)
* 3 [Setup WebCFace Library](https://na-trium-144.github.io/webcface/md_docs_231__setup.html) / [Building from Source](https://na-trium-144.github.io/webcface/md_docs_232__building.html)
* [C/C++ API Reference](https://na-trium-144.github.io/webcface/namespaces.html)
* [webcface-python API Reference](https://na-trium-144.github.io/webcface-python/)
* [webcface-js API Reference](https://na-trium-144.github.io/webcface-js/)

## Links

* [webcface](https://github.com/na-trium-144/webcface): Server & C/C++ client library (this repository)
* [webcface-webui](https://github.com/na-trium-144/webcface-webui): Web browser UI application
* [webcface-tools](https://github.com/na-trium-144/webcface-tools): Command line tools
* [webcface-js](https://github.com/na-trium-144/webcface-js): JavaScript client library
* [webcface-python](https://github.com/na-trium-144/webcface-python): Python client library
* [webcface-package](https://github.com/na-trium-144/webcface-package): Repository for releasing built server, libraries, WebUI, and tools
* [homebrew-webcface](https://github.com/na-trium-144/homebrew-webcface): Manages Homebrew tap
<!-- * [plotjuggler-webcface-plugin](https://github.com/na-trium-144/plotjuggler-webcface-plugin): PlotJuggler plugin -->

## Installation

For WebCFace ver1, refer to the [v1 branch](https://github.com/na-trium-144/webcface/tree/v1?tab=readme-ov-file#installation) (documentation is Japanese only).

For ver2, you can download built archives for Linux, MacOS and Windows as follows, or run it with Docker.
If you want to build from source, refer to [3-2. Building from Source](https://na-trium-144.github.io/webcface/md_docs_232__building.html).

> WebCFace ver1については [v1ブランチ](https://github.com/na-trium-144/webcface/tree/v1?tab=readme-ov-file#installation) を参照してください。
> 
> ver2は以下のようにLinux,Windows,MacOS用にビルドしたアーカイブをダウンロードするか、Dockerで動かすことができます。
> 自分でソースからビルドする場合は [3-2. Building from Source](https://na-trium-144.github.io/webcface/md_docs_232__building.html) を参照してください。

### Linux (x86_64, arm64, armhf) / MacOS (x86_64, arm64)

You can use the following [installation script](installer.sh).
By default, it will install WebCFace using apt-get (on Debian-based systems) or extract the archive manually to /opt/webcface (both on Linux and macOS) and /Applications (on macOS).
(Installation does not occur immediately upon execution, and one or two confirmation prompts will be displayed.)

After installation, you may need to add PATH settings to files such as .bashrc or .zshrc.
Follow the instructions displayed after running the installer.

Linux binaries are built on Ubuntu 20.04, and MacOS binaries are built on Ventura.
So they are not guaranteed to work on older OSs or other distros.

> 以下の[インストールスクリプト](installer.sh)を使ってインストールできます。
> デフォルトでは apt-get でインストールされるか、または手動で /opt/webcface (Linux,MacOS) と /Applications (MacOSのみ) に展開されます。
> (実行すると即座にインストールされるわけではなく、1つか2つ確認のプロンプトが表示されます。)
>
> インストール後、 .bashrc や .zshrc などのファイルにPATHの設定を追加する必要がある場合があります。
> インストーラー実行後に表示される指示に従ってください。
> 
> Linux用バイナリは Ubuntu 20.04 で、 MacOS用バイナリは Ventura でビルドしているため、Ubuntu以外のディストリビューションや古いOSでの動作は保証しません。

```sh
sudo sh -c "$(curl -f https://na-trium-144.github.io/webcface/installer.sh)"
```

If you want to automate the installation, add `--` followed by options such as `-a`/`-x` or `-y` at the end.

> スクリプト内などで呼び出して自動化したい場合は、後ろに `--` と `-a`/`-x` や `-y` などのオプションを追加してください。

```sh
sudo sh -c "$(curl -f https://na-trium-144.github.io/webcface/installer.sh)" -- -a -y
```

```
Usage: installer.sh [-a|-x] [-y] [-d DIR] [-u] [VERSION]
  -a: Install with apt-get (only for Debian-based systems)
  -x: Extract archive manually, without using apt-get
  -d DIR: Extract archive to DIR (default: /opt/webcface)
  -u: Extract webcface-desktop.app to $HOME/Applications, instead of the default /Applications (only for macOS)
  -y: Assume yes for all prompts
  VERSION: Version to install, without 'v' prefix (default: 2.7.0)
           Available versions: 2.7.0 2.5.2-1 2.5.2 2.5.1 2.5.0-2 2.5.0-1 2.5.0 2.4.2 2.4.1-2 2.4.1-1 2.4.1 2.4.0-1 2.4.0 2.3.0 2.2.1 2.2.0 2.1.0 2.0.5 2.0.4
```

<details>

#### Deb Package

You can download and install the Deb package from [WebCFace Releases](https://github.com/na-trium-144/webcface/releases), [webui Releases](https://github.com/na-trium-144/webcface-webui/releases), and [tools Releases](https://github.com/na-trium-144/webcface-tools/releases).
In addition to the contents distributed as zip archives, it includes the WebCFace Desktop application launcher.

> Debパッケージとしてビルドしたものを [WebCFaceのReleases](https://github.com/na-trium-144/webcface/releases) と [webuiのReleases](https://github.com/na-trium-144/webcface-webui/releases) 、[toolsのReleases](https://github.com/na-trium-144/webcface-tools/releases) からダウンロードしてインストールできます。
> 内容はzipアーカイブで配布しているものに加えて WebCFace Desktop のアプリケーションランチャーが含まれます。

<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v2.7.0/webcface_2.7.0_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v2.2.0/webcface-tools_2.2.0_amd64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.12.2/webcface-webui_1.12.2_all.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.12.2/webcface-desktop_1.12.2_linux_amd64.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v2.7.0/webcface_2.7.0_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v2.2.0/webcface-tools_2.2.0_arm64.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.12.2/webcface-webui_1.12.2_all.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.12.2/webcface-desktop_1.12.2_linux_arm64.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface/releases/download/v2.7.0/webcface_2.7.0_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-tools/releases/download/v2.2.0/webcface-tools_2.2.0_armhf.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.12.2/webcface-webui_1.12.2_all.deb
curl -fLO https://github.com/na-trium-144/webcface-webui/releases/download/v1.12.2/webcface-desktop_1.12.2_linux_armv7l.deb
sudo apt install ./webcface*.deb
rm ./webcface*.deb
```
</details>

#### Unzip manually (Linux)

Instead of using apt, you can download the zip file for Linux from the [webcface-package repository](https://github.com/na-trium-144/webcface-package/releases) and extract it to a location of your choice.

* If you have sudo privileges, we recommend extracting it to /usr/local or /opt/webcface.
* Also, add bin/ in the extracted directory to PATH and lib/\*-linux-gnu\*/pkgconfig/ to PKG_CONFIG_PATH.
<!-- * If you want to use the systemd service file, add a link to the file extracted to /opt/webcface/lib/systemd/system in /etc/systemd/system/.
    * If you extract it to a location other than /opt/webcface, you will need to manually rewrite the path written in the service file. -->

> aptを使う代わりに、
> [webcface-package リポジトリから](https://github.com/na-trium-144/webcface-package/releases)
> linux用のzipファイルをダウンロードし、任意の場所に展開して使うこともできます。
> * sudo権限が使える場合は /usr/local または /opt/webcface に展開するのがおすすめです。
> * また、展開したディレクトリ内の bin/ をPATHに、 lib/\*-linux-gnu\*/pkgconfig/ をPKG_CONFIG_PATHに追加してください。
<!-- > * さらにsystemdのサービスファイルを使用したい場合は /opt/webcface/lib/systemd/system に展開されたファイルに対して /etc/systemd/system/ にリンクを貼るなどしてください。
>     * /opt/webcface 以外の場所に展開した場合はserviceファイルに書かれているパスを手動で書き換える必要があります。 -->

<details open><summary>x86_64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.7.0/webcface_2.7.0_linux_amd64.zip
sudo unzip webcface_2.7.0_linux_amd64.zip -d /opt/webcface
rm webcface_2.7.0_linux_amd64.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.bashrc
sudo ln -sf /opt/webcface/lib/systemd/system/*.service /etc/systemd/system/
```
</details>

<details><summary>arm64</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.7.0/webcface_2.7.0_linux_arm64.zip
sudo unzip webcface_2.7.0_linux_arm64.zip -d /opt/webcface
rm webcface_2.7.0_linux_arm64.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.bashrc
sudo ln -sf /opt/webcface/lib/systemd/system/*.service /etc/systemd/system/
```
</details>

<details><summary>armhf</summary>

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.7.0/webcface_2.7.0_linux_armhf.zip
sudo unzip webcface_2.7.0_linux_armhf.zip -d /opt/webcface
rm webcface_2.7.0_linux_armhf.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/arm-linux-gnueabihf/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.bashrc
sudo ln -sf /opt/webcface/lib/systemd/system/*.service /etc/systemd/system/
```
</details>

#### macOS

[From the webcface-package repository](https://github.com/na-trium-144/webcface-package/releases)
You can download a zip file for macOS, but since it is not signed or notarized, it will be blocked by Gatekeeper when downloaded from a browser.
(A screen will appear saying "Cannot open because the developer cannot be verified.")

Instead, you can avoid Gatekeeper by downloading and extracting it from the command line as follows.
(The following command extracts webcface_universal to /opt/webcface and webcface-desktop_app to /Applications, and writes the environment variables to zshrc. Please read appropriately if you are using a different environment.)

* It is a universal binary, and is the same for both IntelMac and Apple Silicon.
* The webcface library has install_name set to the path /opt/webcface/lib, so if you extract it to a location other than that, you may also need to
`export DYLD_LIBRARY_PATH="(webcface directory path)/lib:$DYLD_LIBRARY_PATH"`

> [webcface-package リポジトリから](https://github.com/na-trium-144/webcface-package/releases)
> macos用のzipファイルをダウンロードできますが、
> 署名や公証をしていないためブラウザーからダウンロードするとGatekeeperにブロックされてしまいます。
> (開発元を検証できないため開けません。の画面になります)
> 
> その代わり、以下のようにコマンドラインからダウンロード、展開することでGatekeeperを回避できます。
> (以下のコマンドは webcface_universal を /opt/webcface に、 webcface-desktop_app を /Applications に展開し、環境変数をzshrcに書き込みます。それ以外の環境の場合は適宜読み替えてください)
> 
> * Universalバイナリになっており、IntelMacもAppleシリコンも共通です。
> * webcfaceライブラリはinstall_nameが /opt/webcface/lib のパスになっているため、それ以外の場所に展開した場合は
> `export DYLD_LIBRARY_PATH="(webcfaceディレクトリのパス)/lib:$DYLD_LIBRARY_PATH"`
> も必要になるかもしれません。

```sh
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.7.0/webcface_2.7.0_macos_universal.zip
curl -fLO https://github.com/na-trium-144/webcface-package/releases/download/v2.7.0/webcface-desktop_2.7.0_macos_app.zip
sudo unzip webcface_2.7.0_macos_universal.zip -d /opt/webcface
sudo unzip webcface-desktop_2.7.0_macos_app.zip -d /Applications
rm webcface_2.7.0_macos_universal.zip
rm webcface-desktop_2.7.0_macos_app.zip
echo 'export PATH="/opt/webcface/bin:$PATH"' >> ~/.zshrc
echo 'export PKG_CONFIG_PATH="/opt/webcface/lib/pkgconfig:$PKG_CONFIG_PATH"' >> ~/.zshrc
```

</details>

### Homebrew (MacOS, Linux)

You can install from [na-trium-144/webcface tap](https://github.com/na-trium-144/homebrew-webcface) using brew, but it is not recommended because the webcface built with brew requires many dependent libraries as shared libraries.

Also, if you install with brew, the WebCFace Desktop app is not included.

> [na-trium-144/webcface のtap](https://github.com/na-trium-144/homebrew-webcface) からインストールできますが、
> brewでビルドしたwebcfaceはsharedライブラリとして多数の依存ライブラリが必要になるのであまりおすすめしません。
> 
> また、brewでインストールした場合 WebCFace Desktop アプリは付属しません。

```sh
brew tap na-trium-144/webcface
brew install webcface webcface-webui webcface-tools
```

### Windows MSVC (x86, x64)

You can download the exe or zip file for Windows from the [webcface-package repository](https://github.com/na-trium-144/webcface-package/releases).

* x86 and x64 versions are available.
    * However, the installer and WebCFace Desktop app are always 32-bit.
    * The arm64 binary is not distributed, but it should be possible to build from source even on arm64.
* When you run the exe file, the installer will start.
    * Since it is not signed, it may be blocked by Windows Defender.
    In that case, click "More info" → "Run" to run it.
    * The installation location is by default C:\Program Files\webcface (changeable).
    * Also, the PATH environment variable is set automatically, and a shortcut to WebCFace Desktop is added to the Start menu.
    * Uninstall it like any other app from the Control Panel or Settings app.
* Instead of using the installer, you can download and extract the zip file to any location.
    * To use command-line tools or import it with Meson or CMake, you need to manually add the bin/ in the extracted webcface directory to the PATH environment variable.
* You need to install the [latest Visual C++ Redistributable Package](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-microsoft-visual-c-redistributable-version) if it's not installed.
* It should work on relatively new Windows 10 or later. It has not been tested on older Windows.
* Since it is built with the latest version of Visual Studio 2019, linking to the webcface library from older versions of Visual Studio may not work properly.

MinGW binaries are not distributed at this time (please build from source)


> [webcface-package リポジトリから](https://github.com/na-trium-144/webcface-package/releases)
> windows用のexeファイルまたはzipファイルをダウンロードできます。
> 
> * x86バージョンとx64バージョンがあります。
>     * ただしインストーラーと WebCFace Desktop アプリは32bitになっています
>     * arm64のバイナリは配布していませんが、arm64でもソースからビルドすることはできるはずです。
> * exeファイルは実行するとインストーラーが起動します。
>     * 署名していないため Windows Defender にブロックされるかもしれません。
>     その場合は「詳細情報」→「実行」をクリックして実行してください。
>     * インストール場所はデフォルトで C:\Program Files\webcface になります。(変更可能です)
>     * また、自動的に環境変数のPATHが設定され、スタートメニューにも WebCFace Desktop のショートカットが追加されます。
>     * アンインストールはコントロールパネルや設定アプリから他のアプリと同様にできます。
> * インストーラーを使う代わりに、zipファイルを任意の場所に展開して使用することもできます。
>     * コマンドラインツールやライブラリを使う(Meson,CMakeでインポートする)には、展開したwebcfaceディレクトリの中の bin/ を手動で環境変数のPATHに追加する必要があります。
> * [最新の Visual C++ 再頒布可能パッケージ](https://learn.microsoft.com/ja-jp/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-microsoft-visual-c-redistributable-version)
> がインストールされていない場合はインストールする必要があります。
> * 比較的新しいWindows10以上であれば動作するはずです。古いWindowsでは動作確認していません。
> * いずれも最新バージョンの Visual Studio 2019 でビルドしているため、それよりも古い Visual Studio からwebcfaceライブラリにリンクすると正常動作しないかもしれません。
> 
> MinGW用バイナリは今のところ配布していません(ソースからビルドしてください)

### Docker (x86_64, arm64, armhf)

You can also pull and run `webcface-server` and tools using Docker.
The image is released as [ghcr.io/na-trium-144/webcface-package/webcface-amd64](https://ghcr.io/na-trium-144/webcface-package/webcface-amd64).
For arm64 or armhf, replace `amd64` in the image name.

> Dockerを使って `webcface-server` とコマンドラインツールを実行することもできます。
> Dockerイメージは [ghcr.io/na-trium-144/webcface-package/webcface-amd64](https://ghcr.io/na-trium-144/webcface-package/webcface-amd64) としてリリースしています。
> arm64やarmhfの場合はイメージ名の `amd64` の部分を変更してください。

#### webcface-server

* Example (TCP and Unix Socket)
```sh
docker run --rm \
    -p 7530:7530 \
    -v /tmp/webcface:/tmp/webcface \
    ghcr.io/na-trium-144/webcface-package/webcface-amd64:latest
```

#### webcface-tools

* Example (Unix Socket)
```sh
docker run --rm \
    -v /tmp/webcface:/tmp/webcface \
    ghcr.io/na-trium-144/webcface-package/webcface-amd64:latest \
    webcface-ls
```
* Example (TCP)
```sh
docker run --rm \
    --add-host=host.docker.internal:host-gateway \
    ghcr.io/na-trium-144/webcface-package/webcface-amd64:latest \
    webcface-ls -a host.docker.internal
```

## License

WebCFace and related programs are all released under the MIT license. For details, see [LICENSE](https://github.com/na-trium-144/webcface/blob/main/LICENSE).

Third-party libraries used by WebCFace itself and tools are licensed as follows:

> WebCFaceと関連するプログラムはすべてMITライセンスで公開しています。詳細は [LICENSE](https://github.com/na-trium-144/webcface/blob/main/LICENSE) を参照してください。
> 
> WebCFace本体とtoolsが使用しているサードパーティーのライブラリのライセンスはそれぞれ以下を参照してください。

* Asio (Boost Software License): http://think-async.com/Asio/
* CLI11 (BSD 3-Clause): https://github.com/CLIUtils/CLI11
* Crow (BSD 3-Clause): https://github.com/CrowCpp/Crow
* curl: https://curl.se/docs/copyright.html
* fmt (MIT): https://github.com/fmtlib/fmt
* libvips (LGPL-2.1): https://github.com/libvips/libvips
* msgpack-cxx (Boost Software License): https://github.com/msgpack/msgpack-c
* spdlog (MIT): https://github.com/gabime/spdlog
* UTF8-CPP (BSD 1.0): https://github.com/nemtrif/utfcpp
* FTXUI (MIT): https://github.com/ArthurSonzogni/FTXUI
* tiny-process-library (MIT): https://gitlab.com/eidheim/tiny-process-library
* toml++ (MIT): https://github.com/marzer/tomlplusplus
