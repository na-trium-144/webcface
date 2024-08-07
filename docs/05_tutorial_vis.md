# Tutorial (Visualizing)

\tableofcontents

WebCFaceの機能紹介・チュートリアルです。

このチュートリアルはWebUIでデータを可視化する・WebUIからプログラムを操作するという部分をメインにしたものです。
プロセス間通信に重点をおいたチュートリアルは [Tutorial (Communication)](./06_tutorial_comm.md)

このチュートリアルでは C++ (Meson または CMake)、または Python を使用します。

## 1 WebCFaceのインストール

READMEにしたがって webcface, webcface-webui, webcface-tools をインストールしましょう。

C++の場合はインストールしたwebcfaceパッケージにクライアントライブラリも含まれていますが、
Pythonで利用したい場合は別途 `pip install webcface` でクライアントライブラリをインストールしてください。

## 2 Server

WebCFaceを使用するときはserverを常時立ち上げておく必要があります。
次のように WebCFace-Desktop アプリから、またはコマンドラインからの2つの方法があります。

### 2-1 WebCFace-Desktop

todo

### 2-2 コマンドラインから

ターミナルを開いて
```sh
webcface-server
```
コマンドでサーバーが起動します。
デフォルトでは7530番ポートを開きクライアントの接続を待ちます。

コマンドラインオプションで起動するポートなどを変更できたりします。
Serverの機能については詳細は [Server](./10_server.md)

serverは起動したまま、起動時に表示されるurl (http://IPアドレス:7530/index.html) をブラウザで開くと、
WebCFaceのメイン画面(WebUI)が開きます。
使用しているターミナルによっては、Ctrl(Command)+クリックなどで開くことができるかもしれません。

## 3 WebUI

WebCFaceのメイン画面が起動します。
WebCFaceにクライアントが接続すると、WebUI右上のMenuに表示されます。
Menuから見たいデータを選ぶことで小さいウィンドウのようなものが現れデータを見ることができます。

ウィンドウの表示状態などは自動的にブラウザ(LocalStorage)に保存され、次回アクセスしたときに復元されます。

## 4 Setup

ここからWebCFaceを使ったプログラムを書いていきます。

まずはプロジェクトを作成しWebCFaceライブラリを使えるようにします。

\note
MesonやCMakeを使わない場合、pkg-configを使ったり手動でコンパイル・リンクすることも可能です。
詳細な説明は [Setup](./20_setup.md)

<div class="tabbed">

- <b class="tab-title">C++ (Meson)</b>
    適当にディレクトリを作ります
    ```sh
    mkdir webcface-tutorial
    cd webcface-tutorial
    ```
    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * main.cc
    ```cpp
    #include <iostream>
    #include <webcface/webcface.h>

    int main() {
        webcface::Client wcli("tutorial");
        wcli.start();

        std::cout << "Hello, World!" << std::endl;
    }
    ```
    * meson.build
    ```meson
    project('webcface-tutorial', 'cpp',
      default_options: ['cpp_std=c++17'],
    )
    webcface_dep = dependency('webcface')
    executable('tutorial', 'main.cc',
      dependencies: webcface_dep,
    )
    ```
    
    ビルドして、実行します
    ```sh
    meson setup build  # ← 初回のみ
    meson compile -C build
    ./build/tutorial
    ```
    このチュートリアルでは以降ビルドと実行の手順は省略しますが、
    同様に `meson compile` (またはninja) でビルドして `./build/tutorial` を実行してください。

- <b class="tab-title">C++ (CMake)</b>
    適当にディレクトリを作ります
    ```sh
    mkdir webcface-tutorial
    cd webcface-tutorial
    ```
    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * main.cc
    ```cpp
    #include <iostream>
    #include <webcface/webcface.h>

    int main() {
        webcface::Client wcli("tutorial");
        wcli.start();

        std::cout << "Hello, World!" << std::endl;
    }
    ```
    * CMakeLists.txt
    ```cmake
    cmake_minimum_required(VERSION 3.5)
    project(webcface-tutorial)
    find_package(webcface 2.0 CONFIG REQUIRED)
    add_executable(tutorial main.cc)
    target_link_libraries(tutorial PRIVATE webcface::webcface)
    ```
    
    ビルドして、実行します
    ```sh
    cmake -B build  # ← 初回のみ
    cmake --build build
    ./build/tutorial
    ```
    このチュートリアルでは以降ビルドと実行の手順は省略しますが、
    同様に `cmake --build` (またはmakeやninjaなどでも可) でビルドして `./build/tutorial` を実行してください。

</div>

このプログラムを実行すると、`Hello, World!` と出力されると同時に、
WebUI の右上のメニューの中にClientで指定した名前「tutorial」が現れるはずです。

## 5 データの送信





